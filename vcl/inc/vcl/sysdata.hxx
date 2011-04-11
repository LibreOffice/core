/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#if defined( WNT ) || defined( OS2 )
#include <windef.h>
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
    NSView*               pView;          // the cocoa (NSView *) implementing this object
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
    void*           pXRenderFormat;  // render format for drawable
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
    unsigned long         nSize;         // size in bytes of this structure
    SystemGlyphDataVector rGlyphData;    // glyph data
    int                   orientation;   // Text orientation
};

#endif // _SV_SYSDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
