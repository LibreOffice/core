/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SV_SYSDATA_HXX
#define _SV_SYSDATA_HXX

#include <vector>
#include <cstddef>

#ifdef MACOSX
// predeclare the native classes to avoid header/include problems
typedef struct CGContext *CGContextRef;
typedef struct CGLayer   *CGLayerRef;
#ifdef ENABLE_CORETEXT
typedef const struct __CTFont * CTFontRef;
#endif
#ifdef __OBJC__
@class NSView;
#else
class NSView;
#endif
#endif

#ifdef IOS
typedef const struct __CTFont * CTFontRef;
typedef struct CGContext *CGContextRef;
#endif

#if defined( WNT )
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)
#endif
#include <windef.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

// -----------------
// - SystemEnvData -
// -----------------

struct SystemEnvData
{
    unsigned long       nSize;          // size in bytes of this structure
#if defined( WNT )
    HWND                hWnd;           // the window hwnd
#elif defined( MACOSX )
    NSView*               pView;          // the cocoa (NSView *) implementing this object
#elif defined( ANDROID )
    // Nothing
#elif defined( IOS )
    // Nothing
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

// --------------------
// - SystemParentData -
// --------------------

struct SystemParentData
{
    unsigned long   nSize;            // size in bytes of this structure
#if defined( WNT )
    HWND            hWnd;             // the window hwnd
#elif defined( MACOSX )
    NSView*         pView;            // the cocoa (NSView *) implementing this object
#elif defined( ANDROID )
    // Nothing
#elif defined( IOS )
    // Nothing
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
#elif defined( MACOSX )
    // Nothing
#elif defined( ANDROID )
    // Nothing
#elif defined( IOS )
    // Nothing
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
#elif defined( MACOSX )
    CGContextRef    rCGContext;     // CoreGraphics graphic context
#elif defined( ANDROID )
    // Nothing
#elif defined( IOS )
    // Nothing
#elif defined( UNX )
    void*           pDisplay;       // the relevant display connection
    long            hDrawable;      // a drawable
    void*           pVisual;        // the visual in use
    int         nScreen;        // the current screen of the drawable
    int             nDepth;         // depth of said visual
    long            aColormap;      // the colormap being used
    void*           pXRenderFormat;  // render format for drawable
#endif
    SystemGraphicsData()
        : nSize( sizeof( SystemGraphicsData ) )
#if defined( WNT )
        , hDC( 0 )
#elif defined( MACOSX )
    // Nothing
#elif defined( ANDROID )
    // Nothing
#elif defined( IOS )
    // Nothing
#elif defined( UNX )
        , pDisplay( NULL )
        , hDrawable( 0 )
        , pVisual( NULL )
        , nScreen( 0 )
        , nDepth( 0 )
        , aColormap( 0 )
        , pXRenderFormat( NULL )
#endif
    { }
};


// --------------------
// - SystemWindowData -
// --------------------

struct SystemWindowData
{
    unsigned long   nSize;          // size in bytes of this structure
#if defined( WNT )                  // meaningless on Windows
#elif defined( MACOSX )             // meaningless on Mac OS X
    // Nothing
#elif defined( ANDROID )
    // Nothing
#elif defined( IOS )
    // Nothing
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
#elif defined( MACOSX )
#ifndef ENABLE_CORETEXT
    void*           aATSUFontID;    // native font object
#endif
#elif defined( UNX )
    void*           nFontId;        // native font id
    int             nFontFlags;     // native font flags
#endif
    bool            bFakeBold;      // Does this font need faking the bold style
    bool            bFakeItalic;    // Does this font need faking the italic style
    bool            bAntialias;     // Should this font be antialiased
    bool            bVerticalCharacterType;      // Is the font using vertical character type

    SystemFontData()
        : nSize( sizeof( SystemFontData ) )
#if defined( WNT )
        , hFont( 0 )
#elif defined( MACOSX )
#ifndef ENABLE_CORETEXT
        , aATSUFontID( NULL )
#endif
#elif defined( UNX )
        , nFontId( NULL )
        , nFontFlags( 0 )
#endif
        , bFakeBold( false )
        , bFakeItalic( false )
        , bAntialias( true )
        , bVerticalCharacterType( false )
    {
    }
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
