/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  aquaOpenGLView.m
 */

#import <Foundation/NSObjCRuntime.h>
#if defined (NSFoundationVersionNumber10_5) &&  MAC_OS_X_VERSION_MAX_ALLOWED < 1050
@class CALayer;
@class NSViewController;
typedef int NSColorRenderingIntent;
#endif

#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#include "aquaOpenGLView.h"

@implementation AquaOpenGLView

+ (NSOpenGLPixelFormat*)defaultPixelFormat
{
// first simple implementation (let's see later with more complex )
    NSOpenGLPixelFormatAttribute attributes [] = 
    {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,	// double buffered
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16, // 16 bit depth buffer
        (NSOpenGLPixelFormatAttribute)nil
    };
    return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
}

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat*)format
{
    self = [super initWithFrame:frameRect];
    if (self != nil) {
        _pixelFormat   = [format retain];
    }
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_surfaceNeedsUpdate:) name:NSViewGlobalFrameDidChangeNotification object:self];
    return self;
}

- (void)dealloc
{   // get rid of the context and pixel format
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSViewGlobalFrameDidChangeNotification object:self];
    [self clearGLContext];
    if (_pixelFormat)
        [_pixelFormat release];

    [super dealloc];
}

- (void)setOpenGLContext:(NSOpenGLContext*)context
{
    [self clearGLContext];
    _openGLContext = [context retain];
}

- (NSOpenGLContext*)openGLContext
{   // create a context the first time through
     if (_openGLContext == NULL) {
        _openGLContext = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat != nil ? _pixelFormat : [[self class] defaultPixelFormat] shareContext:nil];
        [_openGLContext makeCurrentContext];
        [self prepareOpenGL]; // call to initialize OpenGL state here
    }
    return _openGLContext;
}

- (void)clearGLContext
{
    if (_openGLContext != nil) {
        if ([_openGLContext view] == self) {
            [_openGLContext clearDrawable];
        }
        [_openGLContext release];
        _openGLContext = nil;
    }
}

- (void)prepareOpenGL
{
    // for overriding to initialize OpenGL state, occurs after context creation
#if MACOSX_SDK_VERSION < 1050
    long swapInt = 1;
#else
    GLint swapInt = 1;
#endif


    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval]; // set to vbl sync

    // init GL stuff here
    // FIXME: why is there garbage using prepareOpenGL ,
    // but NOT, wen using the same content,
    // directly in the OGLTrans instance ?
    glShadeModel( GL_SMOOTH );
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);  // R G B A
    glClear(GL_COLOR_BUFFER_BIT);
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    glEnable(GL_TEXTURE_2D);


    glEnable(GL_LIGHTING);
    GLfloat light_direction[] = { 0.0 , 0.0 , 1.0 };
    GLfloat materialDiffuse[] = { 1.0 , 1.0 , 1.0 , 1.0};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,materialDiffuse);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    [[self openGLContext] flushBuffer];
}

- (BOOL)isOpaque
{
    return YES;
}

- (void)drawRect
{
    // get context. will create if we don't have one yet
    NSOpenGLContext* context = [self openGLContext];
    [context makeCurrentContext];
    //perform drawing here
    [context flushBuffer];
}

- (void)lockFocus
{
    // get context. will create if we don't have one yet
    NSOpenGLContext* context = [self openGLContext];
    
    // make sure we are ready to draw
    [super lockFocus];

    // when we are about to draw, make sure we are linked to the view
    if ([context view] != self) {
        [context setView:self];
    }

    // make us the current OpenGL context
    [context makeCurrentContext];
}

// no reshape will be called since NSView does not export a specific reshape method

- (void)update
{
    if ([_openGLContext view] == self) {
        [_openGLContext update];
    }
}

- (void) _surfaceNeedsUpdate:(NSNotification*)notification
{
    (void) notification; // unused
    [self update];
}

- (void)setPixelFormat:(NSOpenGLPixelFormat*)pixelFormat
{
    [_pixelFormat release];
    _pixelFormat = [pixelFormat retain];
}

- (NSOpenGLPixelFormat*)pixelFormat
{
    return _pixelFormat;
}


- (void)encodeWithCoder:(NSCoder *)coder 
{

    [super encodeWithCoder:coder];
    if (![coder allowsKeyedCoding]) {
        [coder encodeValuesOfObjCTypes:"@iii", &_pixelFormat];
    } else {
        [coder encodeObject:_pixelFormat forKey:@"NSPixelFormat"];
    }
}

- (id)initWithCoder:(NSCoder *)coder 
{

    self = [super initWithCoder:coder];

    if (![coder allowsKeyedCoding]) {
        [coder decodeValuesOfObjCTypes:"@iii", &_pixelFormat];
    } else {
        _pixelFormat = [[coder decodeObjectForKey:@"NSPixelFormat"] retain];
    }
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_surfaceNeedsUpdate:) name:NSViewGlobalFrameDidChangeNotification object:self];
    
    return self;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
