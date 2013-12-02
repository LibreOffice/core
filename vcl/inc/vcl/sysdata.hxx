/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_SYSDATA_HXX
#define _SV_SYSDATA_HXX

#include <vector>

#ifdef QUARTZ
// predeclare the native classes to avoid header/include problems
typedef struct CGContext *CGContextRef;
typedef struct CGLayer   *CGLayerRef;
#ifdef __OBJC__
@class NSView;
#else
class NSView;
#endif
#endif

// -----------------
// - SystemEnvData -
// -----------------

struct SystemEnvData
{
    unsigned long       nSize;          // size in bytes of this structure
#if defined( WNT ) || defined( OS2 )
    HWND                hWnd;           // the window hwnd
#elif defined( QUARTZ )
    NSView*             mpNSView;       // the cocoa (NSView *) implementing this object
#elif defined( UNX )
    void*               pDisplay;       // the relevant display connection
    long                aWindow;        // the window of the object
    void*               pSalFrame;      // contains a salframe, if object has one
    void*               pWidget;        // the corresponding widget
    void*               pVisual;        // the visual in use
    int             nScreen;        // the current screen of the window
    int                 nDepth;         // depth of said visual
    long                aColormap;      // the colormap being used
    void*               pAppContext;    // the application context in use
    long                aShellWindow;   // the window of the frame's shell
    void*               pShellWidget;   // the frame's shell widget
#endif
};

#define SystemChildData SystemEnvData

// --------------------
// - SystemParentData -
// --------------------

struct SystemParentData
{
    unsigned long   nSize;            // size in bytes of this structure
#if defined( WNT ) || defined( OS2 )
    HWND            hWnd;             // the window hwnd
#elif defined( QUARTZ )
    NSView*         pView;            // the cocoa (NSView *) implementing this object
#elif defined( UNX )
    long            aWindow;          // the window of the object
    bool            bXEmbedSupport:1; // decides whether the object in question
                                      // should support the XEmbed protocol
#endif
};

// --------------------
// - SystemMenuData -
// --------------------

struct SystemMenuData
{
    unsigned long   nSize;          // size in bytes of this structure
#if defined( WNT )
    HMENU           hMenu;          // the menu handle of the menu bar
#elif defined( QUARTZ )
    //not defined
#elif defined( UNX )
    long            aMenu;          // ???
#endif
};

// --------------------
// - SystemGraphicsData -
// --------------------

struct SystemGraphicsData
{
    unsigned long   nSize;          // size in bytes of this structure
#if defined( WNT )
    HDC             hDC;            // handle to a device context
#elif defined( QUARTZ )
    CGContextRef    rCGContext;     // QUARTZ graphic context
#elif defined( UNX )
    void*           pDisplay;       // the relevant display connection
    long            hDrawable;      // a drawable
    void*           pVisual;        // the visual in use
    int         nScreen;        // the current screen of the drawable
    int             nDepth;         // depth of said visual
    long            aColormap;      // the colormap being used
    void*           pRenderFormat;  // render format for drawable
#endif
};


// --------------------
// - SystemWindowData -
// --------------------

struct SystemWindowData
{
    unsigned long   nSize;          // size in bytes of this structure
#if defined( WNT )                  // meaningless on Windows
#elif defined( QUARTZ )             // meaningless on Mac OS X / Quartz
#elif defined( UNX )
    void*           pVisual;        // the visual to be used
#endif
};


// --------------------
// - SystemGlyphData -
// --------------------

struct SystemGlyphData
{
    unsigned long        index;
    double               x;
    double               y;
    int                  fallbacklevel;
};


// --------------------
// - SystemFontData -
// --------------------

struct SystemFontData
{
    unsigned long   nSize;          // size in bytes of this structure
#if defined( WNT )
    HFONT           hFont;          // native font object
#elif defined( OS2 )
    ULONG           hFont;          // native font object
#elif defined( QUARTZ )
    void*           aATSUFontID;    // native font object
#elif defined( UNX )
    void*           nFontId;        // native font id
    int             nFontFlags;     // native font flags
#endif
    bool            bFakeBold;      // Does this font need faking the bold style
    bool            bFakeItalic;    // Does this font need faking the italic style
    bool            bAntialias;     // Should this font be antialiased
    bool            bVerticalCharacterType;      // Is the font using vertical character type
};

// --------------------
// - SystemTextLayoutData -
// --------------------

typedef std::vector<SystemGlyphData> SystemGlyphDataVector;

struct SystemTextLayoutData
{
    unsigned long   nSize;                      // size in bytes of this structure
    std::vector<SystemGlyphData> rGlyphData;    // glyph data
    int             orientation;                // Text orientation
};

#endif // _SV_SYSDATA_HXX

