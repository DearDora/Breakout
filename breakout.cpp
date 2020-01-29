#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <vector>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
using namespace std;

const int Border = 2;

// Game information
int FPS = 70;
int SPEED = 70;
int storeSpeed;
int SCORE = -10;
int BreakNum = 0;
int mapNum = 1;
bool gamePause = false;
bool buffExist = false;
// X11 structures
struct XInfo {
    Display*  display;
    Window   window;
    GC       gc;
};
Colormap screen_colormap;
XColor red, orange, yellow, green, blue, purple, brown, gray, black;
Status rc;

// Structure of Breaks
struct Break{
    XPoint position;
    XColor color;
    bool on = true;
};

// Structure of Ball
struct Ball{
    int size = 15;
    XPoint position = {640,685};
    XPoint direction = {5,5};
    XColor color = black;
};

// Structure of Paddle
struct Paddle{
    int length = 256;
    XPoint position = {640,700};
    XColor color = gray;
};

// Sturcture of Buff ball
struct Buff{
    int size = 10;
    XPoint position;
    XPoint direction = {0,10};
    XColor color;
    bool isOn = false;
};

// Function to put out a message on error exits.
void error(string s) {
    cerr << s << endl;
    exit(-1);
}

// get current time
unsigned long now() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

//  Create the window;  initialize X.
void initX(XInfo & xinfo){
    // create window
    xinfo.display = XOpenDisplay("");
    if (!xinfo.display){
        error("ERROR: Cannot open display");
    }
    
    int screennum = DefaultScreen(xinfo.display);
    unsigned long background = WhitePixel(xinfo.display, screennum);
    unsigned long foreground = BlackPixel(xinfo.display, screennum);
    
    XSizeHints screen;
    screen.x = 0;
    screen.y = 0;
    screen.width = 1280;
    screen.height = 800;
    screen.flags = PPosition | PSize;
    xinfo.window = XCreateSimpleWindow(xinfo.display, DefaultRootWindow( xinfo.display ), screen.x, screen.y, screen.width, screen.height, Border, foreground, background);
    
    xinfo.gc = XCreateGC (xinfo.display, xinfo.window, 0, 0 );
    XSetBackground( xinfo.display, xinfo.gc, background );
    XSetForeground( xinfo.display, xinfo.gc, foreground );
    
    // Tell the base window system what input events you want.
    XSelectInput( xinfo.display, xinfo.window,
                 ButtonPressMask | KeyPressMask | ButtonMotionMask );
    
    XMapRaised( xinfo.display, xinfo.window );
    XFlush(xinfo.display);
}

// Set the font
void setFont(XInfo xinfo, int no){
    XFontStruct * font;
    const char * fontname;
    if(no == 1){
        fontname = "-*-helvetica-Bold-r-*-*-32-*-*-*-*-*-*-*";
    }else if(no == 2){
        fontname = "-*-helvetica-*-r-*-*-18-*-*-*-*-*-*-*";
    }else if(no == 3){
        fontname = "-*-times-*-r-*-*-18-*-*-*-*-*-*-*";
    }else if(no == 4){
        fontname = "-*-times-Bold-r-*-*-24-*-*-*-*-*-*-*";
    }
    font = XLoadQueryFont (xinfo.display, fontname);
    XSetFont (xinfo.display, xinfo.gc, font->fid);
}

// Draw words
void drawString(XInfo xinfo, int x, int y, string & s){
    XDrawImageString(xinfo.display, xinfo.window, xinfo.gc, x, y, s.c_str(), s.length());
}

// Display the info page
void infoPage(){
    XInfo xinfo;
    initX(xinfo);
    
    usleep(10*1000);
    // text messages
    string text1("ASSIGNMENT 1: BREAKOUT");
    string text2("NAME: Shuo Diao");
    string text3("USERID: 20639162");
    string text4("PRESS [A] to move left");
    string text5("PRESS [D] to move right");
    string text6("PRESS [P] to pause");
    string text7("PRESS [C] to continue");
    string text8("PRESS [R] to restart");
    string text9("PRESS [Q] to exit");
    string text10("Press [ANY KEY] to start");
    // Display messages
    setFont(xinfo, 1);
    drawString(xinfo, 425, 250, text1);
    setFont(xinfo, 2);
    drawString(xinfo, 1000, 700, text2);
    drawString(xinfo, 1000, 725, text3);
    setFont(xinfo, 3);
    drawString(xinfo, 450, 350, text4);
    drawString(xinfo, 450, 375, text5);
    drawString(xinfo, 450, 400, text6);
    drawString(xinfo, 450, 425, text7);
    drawString(xinfo, 450, 450, text8);
    drawString(xinfo, 450, 475, text9);
    setFont(xinfo, 4);
    drawString(xinfo, 525, 600, text10);
    
    XFlush(xinfo.display);
    
    XEvent event;
    // Press any key to close this window
    bool start = true;
    while(start){
        XNextEvent(xinfo.display, &event );
        if(event.type == KeyPress){
            start = false;
            break;
        }
    }
    XCloseDisplay(xinfo.display);
}

void setColor(XInfo& xinfo){
    /* get access to the screen's color map. */
    screen_colormap = DefaultColormap(xinfo.display, DefaultScreen(xinfo.display));
    
    /* allocate the set of colors we will want to use for the drawing. */
    rc = XAllocNamedColor(xinfo.display, screen_colormap, "red", &red, &red);
    if (rc == 0) {
        error("ERROR: XAllocNamedColor - failed to allocated 'red' color.\n");
    }
    rc = XAllocNamedColor(xinfo.display, screen_colormap, "orange", &orange, &orange);
    if (rc == 0) {
        error("ERROR: XAllocNamedColor - failed to allocated 'orange' color.\n");
    }
    rc = XAllocNamedColor(xinfo.display, screen_colormap, "yellow", &yellow, &yellow);
    if (rc == 0) {
        error("ERROR: XAllocNamedColor - failed to allocated 'yellow' color.\n");
    }
    rc = XAllocNamedColor(xinfo.display, screen_colormap, "green", &green, &green);
    if (rc == 0) {
        error("ERROR: XAllocNamedColor - failed to allocated 'green' color.\n");
    }
    rc = XAllocNamedColor(xinfo.display, screen_colormap, "blue", &blue, &blue);
    if (rc == 0) {
        error("ERROR: XAllocNamedColor - failed to allocated 'blue' color.\n");
    }
    rc = XAllocNamedColor(xinfo.display, screen_colormap, "purple", &purple, &purple);
    if (rc == 0) {
        error("ERROR: XAllocNamedColor - failed to allocated 'purple' color.\n");
    }
    rc = XAllocNamedColor(xinfo.display, screen_colormap, "brown", &brown, &brown);
    if (rc == 0) {
        error("ERROR: XAllocNamedColor - failed to allocated 'brown' color.\n");
    }
    rc = XAllocNamedColor(xinfo.display, screen_colormap, "gray", &gray, &gray);
    if (rc == 0) {
        error("ERROR: XAllocNamedColor - failed to allocated 'gray' color.\n");
    }
    rc = XAllocNamedColor(xinfo.display, screen_colormap, "black", &black, &black);
    if (rc == 0) {
        error("ERROR: XAllocNamedColor - failed to allocated 'black' color.\n");
    }
}

void game();

// Changes after catched the Buff ball
void dropBuff(Ball & ball, Paddle & paddle, Buff & buff){
    int num;
    num = rand() % 6 + 1;
    if(num == 1){// Shorter the paddle
        paddle.length = paddle.length - 40;
        paddle.color = brown;
    }else if(num == 2){// Longer the paddle
        paddle.length = paddle.length + 40;
        paddle.color = brown;
    }else if(num == 3){// Larger the size of the ball
        SPEED = 60;
        ball.size = 30;
    }else if(num == 4){// Restore the settings
        ball.size = 15;
        SPEED = storeSpeed;
        paddle.length = 256;
        ball.color = black;
        paddle.color = gray;
    }else if(num == 5){// Change the speed
        SPEED = rand()% 50 + 50;
    } else if(num == 6){// Deduct the score
        SCORE -= 100;
    }
}

// All the animation during the game
void animation(XInfo xinfo, Ball & ball, Paddle & paddle, vector<Break> & breaks, Buff & buff){
    if(breaks.size() == BreakNum){
        breaks.clear();
        BreakNum = 0;
        // WINNING message
        string message("YOU WIN! **PRESS [S] TO START A NEW GAME. PRESS [Q] TO QUIT**");
        XDrawImageString(xinfo.display, xinfo.window, xinfo.gc, 300, 400, message.c_str(), message.length());
        XFlush(xinfo.display);
        
        while(true){
            XEvent event;
            XNextEvent(xinfo.display, &event );
            
            switch (event.type) {
                    
                case KeyPress: // any keypress
                    KeySym key;
                    char text[10];
                    int i = XLookupString((XKeyEvent*)&event, text, 10, &key, 0);
                    
                    // start new game
                    if (i == 1 && text[0] == 's') {
                        XCloseDisplay(xinfo.display);
                        // Random the map
                        srand (time(NULL));
                        mapNum = rand() % 4 + 1;
                        game();
                    }
                    
                    // quit game
                    if ( i == 1 && text[0] == 'q' ) {
                        breaks.clear();
                        XCloseDisplay(xinfo.display);
                        exit(0);
                    }
                    break;
            }
        }
    }
    // update ball position
    ball.position.x += ball.direction.x;
    ball.position.y += ball.direction.y;
    // update buff if it is on
    if(buff.isOn && buffExist){
        buff.position.y += buff.direction.y;
    }
    // Check the breaks
    int count = 0;
    int index = 0;
    int bn;
    for(auto& b: breaks){
        if(b.on == true &&
           ball.position.x - ball.size/2 <= b.position.x + 64 &&
            ball.position.x + ball.size/2 >= b.position.x &&
            ball.position.y + ball.size/2 >= b.position.y &&
            ball.position.y - ball.size/2 <= b.position.y + 32 ){
            
            if(ball.position.x - ball.size/2 < b.position.x + 64 &&
               ball.position.x + ball.size/2 > b.position.x &&
               ball.position.y > b.position.y &&
               ball.position.y < b.position.y + 32){
                index = 1;
            }else if(ball.position.x < b.position.x + 64 &&
                     ball.position.x > b.position.x &&
                     ball.position.y + ball.size/2 > b.position.y &&
                     ball.position.y - ball.size/2 < b.position.y + 32){
                index = 2;
            }else{
                index = 3;
            }
            b.on = false;
            SCORE += 5;
            // Drop the buff ball from the break
            bn = rand() % 6 + 1;
            if(bn == 1 && !buffExist){
                buff.position = b.position;
                buff.position.x += 32;
                buff.position.y += 16;
                buff.isOn = true;
                buff.color = b.color;
                buffExist = true;
            }
            BreakNum ++;
        }
        ++ count;
    }
    // Change the ball direction
    if(index == 1){
        ball.direction.x = -ball.direction.x;
    }else if(index == 2){
        ball.direction.y = -ball.direction.y;
    }else if(index == 3){
        ball.direction.x = -ball.direction.x;
        ball.direction.y = -ball.direction.y;
    }
    
    // bounce ball
    if (ball.position.x + ball.size/2 >= 1280 ||
        ball.position.x - ball.size/2 <= 0)
        ball.direction.x = -ball.direction.x;
    if (ball.position.y - ball.size/2 <= 0)
        ball.direction.y = -ball.direction.y;
    // Ball touched the ground
    if(ball.position.y + ball.size/2 >= 800){
        breaks.clear();
        SCORE = 0;
        BreakNum = 0;
        // WINNING message
        string message("GAME OVER! **PRESS [S] TO START A NEW GAME. PRESS [Q] TO QUIT**");
        XDrawImageString(xinfo.display, xinfo.window, xinfo.gc, 300, 400, message.c_str(), message.length());
        XFlush(xinfo.display);
        
        while(true){
            XEvent event;
            XNextEvent(xinfo.display, &event );
            
            switch (event.type) {
                    
                case KeyPress: // any keypress
                    KeySym key;
                    char text[10];
                    int i = XLookupString((XKeyEvent*)&event, text, 10, &key, 0);
                    
                    // start new game
                    if (i == 1 && text[0] == 's') {
                        XCloseDisplay(xinfo.display);
                        // Random the map
                        srand (time(NULL));
                        mapNum = rand() % 4 + 1;
                        game();
                    }
                    
                    // quit game
                    if ( i == 1 && text[0] == 'q' ) {
                        breaks.clear();
                        XCloseDisplay(xinfo.display);
                        exit(0);
                    }
                    break;
            }
        }
    }
    // Paddle catched the buff ball
    if(buffExist && buff.isOn && buff.position.y + buff.size/2 > 700 && buff.position.y + buff.size/2 < 715 &&
       buff.position.x >= paddle.position.x && buff.position.x <= paddle.position.x+paddle.length){
        buff.isOn = false;
        buffExist = false;
        dropBuff(ball,paddle,buff);
    }else if(buffExist && buff.isOn && buff.position.y + buff.size/2 > 800){
        buffExist = false;
    }
    // Paddle catched the ball
    if(ball.position.y + ball.size/2 > 700 && ball.position.y + ball.size/2 < 715
       && ball.position.x >= paddle.position.x && ball.position.x <= paddle.position.x+paddle.length){
        int n = paddle.length/6;
        if(ball.position.x <= paddle.position.x+ 2*n && ball.position.x >= paddle.position.x+ n){
            ball.direction.x = -5;
            ball.direction.y = -5;
        }else if(ball.position.x <= paddle.position.x+n && ball.position.x >= paddle.position.x){
            ball.direction.x = -6;
            ball.direction.y = -3;
        }else if(ball.position.x <= paddle.position.x+3*n && ball.position.x >= paddle.position.x+2*n){
            ball.direction.x = -3;
            ball.direction.y = -6;
        }else if(ball.position.x <= paddle.position.x+4*n && ball.position.x >= paddle.position.x+2*n){
            ball.direction.x = 3;
            ball.direction.y = -6;
        }else if(ball.position.x <= paddle.position.x+5*n && ball.position.x >= paddle.position.x+4*n){
            ball.direction.x = 5;
            ball.direction.y = -5;
        }else if(ball.position.x <= paddle.position.x+paddle.length && ball.position.x >= paddle.position.x+5*n){
            ball.direction.x = 6;
            ball.direction.y = -3;
        }
        SCORE += 10;
    }
    
}

// Repaint the window
void repaint(XInfo& xinfo, Ball& ball,  Paddle& paddle, vector<Break>& breaks, Buff & buff){
    // clear background
    XClearWindow(xinfo.display, xinfo.window);
    
    // draw score
    string num("SCORE " + to_string(SCORE));
    // Display messages
    XDrawImageString(xinfo.display, xinfo.window, xinfo.gc, 10, 25, num.c_str(), num.length());
    XFlush(xinfo.display);
    // draw FPS
    string num2("FPS " + to_string(FPS));
    // Display messages
    XDrawImageString(xinfo.display, xinfo.window, xinfo.gc, 10, 50, num2.c_str(), num2.length());
    XFlush(xinfo.display);
    // draw buff ball
    if(buff.isOn){
        XSetForeground(xinfo.display, xinfo.gc, black.pixel);
        XDrawArc(xinfo.display, xinfo.window, xinfo.gc, buff.position.x - buff.size/2, buff.position.y - buff.size/2, buff.size, buff.size, 0, 360*64);
        XSetForeground(xinfo.display, xinfo.gc, buff.color.pixel);
        XFillArc(xinfo.display, xinfo.window, xinfo.gc, buff.position.x - buff.size/2, buff.position.y - buff.size/2, buff.size, buff.size, 0, 360*64);
    }
    usleep(1);
    // draw breaks
    for(auto& b: breaks){
        if(b.on == false) continue;
        XSetLineAttributes(xinfo.display, xinfo.gc,
                           4, LineSolid, CapRound, JoinRound);
        XSetForeground(xinfo.display, xinfo.gc, black.pixel);
        XDrawRectangle(xinfo.display,xinfo.window, xinfo.gc, b.position.x, b.position.y, 59, 27);
        XSetForeground(xinfo.display, xinfo.gc, b.color.pixel);
        XFillRectangle(xinfo.display, xinfo.window, xinfo.gc, b.position.x+1, b.position.y+1, 58, 26);
    }
    XFlush(xinfo.display);
    // draw paddle
    XSetForeground(xinfo.display, xinfo.gc, black.pixel);
    XDrawRectangle(xinfo.display, xinfo.window, xinfo.gc, paddle.position.x, paddle.position.y, paddle.length, 8);
    XSetForeground(xinfo.display, xinfo.gc, paddle.color.pixel);
    XFillRectangle(xinfo.display, xinfo.window, xinfo.gc, paddle.position.x+1, paddle.position.y+1, paddle.length-1, 7);
    // draw ball from centre
    XSetForeground(xinfo.display, xinfo.gc, ball.color.pixel);
    XFillArc(xinfo.display, xinfo.window, xinfo.gc, ball.position.x - ball.size/2, ball.position.y - ball.size/2, ball.size, ball.size, 0, 360*64);
    
    XFlush(xinfo.display);
}
// Different breaks making patterns
void makeBreaks(vector<Break> & breaks, int index, short pos, XColor & color){
    for(int i=128; i < 1152 ; i+=index){
        Break tempBreak;
        XPoint tempPos = {static_cast<short>(i),pos};
        tempBreak.position = tempPos;
        tempBreak.color = color;
        breaks.emplace_back(tempBreak);
    }
}
void makeBreaks2(vector<Break> & breaks, int index, short pos, XColor & color){
    for(int i=192; i < 1152 ; i+=index){
        Break tempBreak;
        XPoint tempPos = {static_cast<short>(i),pos};
        tempBreak.position = tempPos;
        tempBreak.color = color;
        breaks.emplace_back(tempBreak);
    }
}
void makeBreaks3(vector<Break> & breaks, int index, short pos, XColor & color){
    for(int i=192; i < 640 ; i+=index){
        Break tempBreak;
        XPoint tempPos = {static_cast<short>(i),pos};
        tempBreak.position = tempPos;
        tempBreak.color = color;
        breaks.emplace_back(tempBreak);
    }
}
void makeBreaks4(vector<Break> & breaks, int index, short pos, XColor & color){
    for(int i=640; i < 1152 ; i+=index){
        Break tempBreak;
        XPoint tempPos = {static_cast<short>(i),pos};
        tempBreak.position = tempPos;
        tempBreak.color = color;
        breaks.emplace_back(tempBreak);
    }
}

// Different maps
void map1(vector<Break> & breaks, int index){
    makeBreaks(breaks,index,96,red);
    makeBreaks(breaks,index,128,orange);
    makeBreaks(breaks,index,160,yellow);
    makeBreaks(breaks,index,192,green);
    makeBreaks(breaks,index,224,blue);
    makeBreaks(breaks,index,256,purple);
}

void map2(vector<Break> & breaks, int index){
    makeBreaks(breaks,index,96,red);
    makeBreaks2(breaks,index,128,orange);
    makeBreaks(breaks,index,160,yellow);
    makeBreaks2(breaks,index,192,green);
    makeBreaks(breaks,index,224,blue);
    makeBreaks2(breaks,index,256,purple);
}

void map3(vector<Break> & breaks, int index){
    makeBreaks3(breaks,index,96,red);
    makeBreaks4(breaks,index,128,orange);
    makeBreaks3(breaks,index,160,yellow);
    makeBreaks4(breaks,index,192,green);
    makeBreaks3(breaks,index,224,blue);
    makeBreaks4(breaks,index,256,purple);
}

void controller(XInfo & xinfo, XEvent & event, Paddle & paddle, XWindowAttributes & w){
    XNextEvent(xinfo.display, &event );
    
    switch (event.type) {
            
        case KeyPress: // any keypress
            KeySym key;
            char text[10];
            int i = XLookupString((XKeyEvent*)&event, text, 10, &key, 0);
            
            // move left
            if (i == 1 && text[0] == 'a') {
                if(paddle.position.x - 40 >= 0)
                    paddle.position.x -= 40;
            }
            
            // move right
            if (i == 1 && text[0] == 'd') {
                if(paddle.position.x + paddle.length <= w.width - 40)
                    paddle.position.x += 40;
            }
            
            // pause game
            if (i == 1 && text[0] == 'p') {
                gamePause = true;
            }
            
            // continue game
            if (i == 1 && text[0] == 'c') {
                gamePause = false;
            }
            
            // restart game
            if (i == 1 && text[0] == 'r') {
                XCloseDisplay(xinfo.display);
                SCORE = 0;
                game();
            }
            
            // quit game
            if ( i == 1 && text[0] == 'q' ) {
                XCloseDisplay(xinfo.display);
                exit(0);
            }
            break;
    }
}

// Load maps
void loadmap(vector<Break> & breaks){
    if (mapNum == 1){
        map1(breaks, 64);
    } else if(mapNum == 2){
        map2(breaks, 128);
    } else if(mapNum == 3){
        map3(breaks, 64);
    } else {
        map1(breaks,128);
    }
}

// Game play
void game(){
    XInfo xinfo;
    initX(xinfo);
    XEvent event;

    setColor(xinfo);
    setFont(xinfo, 2);
    
    Ball ball;
    Paddle paddle;
    Buff buff;
    vector<Break> breaks;
    
    loadmap(breaks);
    
    XWindowAttributes w;
    XGetWindowAttributes(xinfo.display, xinfo.window, &w);
    
    // save time of last window paint
    unsigned long lastRepaint = 0;
    unsigned long lastMove = 0;
    
    // event handle for current event
    // event loop
    while (true) {
        
        // process if we have any events
        if (XPending(xinfo.display) > 0) {
            controller(xinfo,event,paddle, w);
        }
        
        unsigned long end = now();    // get current time in microsecond
        
        if (end - lastRepaint > 1000000 / FPS) {
            repaint(xinfo,ball,paddle,breaks,buff);
            lastRepaint = now(); // remember when the paint happened
        }
        
        if (end - lastMove > 1000000 / SPEED && !gamePause){
            animation(xinfo,ball,paddle,breaks,buff);
            lastMove = now();
        }
        
        // IMPORTANT: sleep for a bit to let other processes work
        if (XPending(xinfo.display) == 0) {
            usleep(1000000 / FPS - (end - lastRepaint));
        }
    }
    XCloseDisplay(xinfo.display);
}

int main (int argc, char* argv[] ) {
    // Use default FPS and SPEED, or customize the FPS and SPEED
    if(argc == 1){
    }else if(argc == 3){
        FPS = stoi(argv[1]) + 40;
        SPEED =  5* stoi(argv[2])+40;
        storeSpeed = SPEED;
    }else{
        error("ERROR: Only allows 0 or 2 arguments.");
    }
    infoPage();
    game();
}
