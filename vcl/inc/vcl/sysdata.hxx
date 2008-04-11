/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sysdata.hxx,v $
 * $Revision: 1.8 $
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

// -----------------
// - SystemEnvData -
// -----------------

struct SystemEnvData
{
    unsigned long       nSize;          // size in bytes of this structure
#if defined( WNT ) || defined( OS2 )
    HWND                hWnd;           // the window hwnd
#elif defined( QUARTZ )
    NSView*             pView;          // the cocoa view ptr implementing this object
#elif defined( UNX )
    void*               pDisplay;       // the relevant display connection
    long                aWindow;        // the window of the object
    void*               pSalFrame;      // contains a salframe, if object has one
    void*               pWidget;        // the corresponding widget
    void*               pVisual;        // the visual in use
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
    NSView*         pView;            // the cocoa view ptr implementing this object
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
    CGContextRef            rCGContext;     // QUARTZ graphic context
#elif defined( UNX )
    long            hDrawable;      // a drawable
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

#endif // _SV_SYSDATA_HXX
