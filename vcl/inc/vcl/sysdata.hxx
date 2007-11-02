/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sysdata.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:43:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SYSDATA_HXX
#define _SV_SYSDATA_HXX

#if defined( QUARTZ )
#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
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
    WindowRef           rWindow;        // Window reference
#endif

#if defined( UNX )
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
    WindowRef       rWindow;        // Window reference
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
