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

#ifndef _SV_DECOVIEW_HXX
#define _SV_DECOVIEW_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/symbol.hxx>

class Rectangle;
class Point;
class Color;
class OutputDevice;

// ------------------------
// - DecorationView-Types -
// ------------------------

// Flags for DrawSymbol()
#define SYMBOL_DRAW_MONO                    ((sal_uInt16)0x0001)
#define SYMBOL_DRAW_DISABLE                 ((sal_uInt16)0x0002)

// Flags for DrawFrame()
#define FRAME_DRAW_IN                       ((sal_uInt16)0x0001)
#define FRAME_DRAW_OUT                      ((sal_uInt16)0x0002)
#define FRAME_DRAW_GROUP                    ((sal_uInt16)0x0003)
#define FRAME_DRAW_DOUBLEIN                 ((sal_uInt16)0x0004)
#define FRAME_DRAW_DOUBLEOUT                ((sal_uInt16)0x0005)
#define FRAME_DRAW_MENU                     ((sal_uInt16)0x0010)
#define FRAME_DRAW_WINDOWBORDER             ((sal_uInt16)0x0020)
#define FRAME_DRAW_BORDERWINDOWBORDER       ((sal_uInt16)0x0040)
#define FRAME_DRAW_MONO                     ((sal_uInt16)0x1000)
#define FRAME_DRAW_NODRAW                   ((sal_uInt16)0x8000)
#define FRAME_DRAW_STYLE                    ((sal_uInt16)0x000F)

// Flags for DrawHighlightFrame()
#define FRAME_HIGHLIGHT_IN                  ((sal_uInt16)0x0001)
#define FRAME_HIGHLIGHT_OUT                 ((sal_uInt16)0x0002)
#define FRAME_HIGHLIGHT_TESTBACKGROUND      ((sal_uInt16)0x4000)
#define FRAME_HIGHLIGHT_STYLE               ((sal_uInt16)0x000F)

// Flags for DrawButton()
#define BUTTON_DRAW_DEFAULT                 ((sal_uInt16)0x0001)
#define BUTTON_DRAW_NOLIGHTBORDER           ((sal_uInt16)0x0002)
#define BUTTON_DRAW_PRESSED                 ((sal_uInt16)0x0004)
#define BUTTON_DRAW_CHECKED                 ((sal_uInt16)0x0008)
#define BUTTON_DRAW_DONTKNOW                ((sal_uInt16)0x0010)
#define BUTTON_DRAW_MONO                    ((sal_uInt16)0x0020)
#define BUTTON_DRAW_NOFILL                  ((sal_uInt16)0x0040)
#define BUTTON_DRAW_DISABLED                ((sal_uInt16)0x0080)
#define BUTTON_DRAW_HIGHLIGHT               ((sal_uInt16)0x0100)
#define BUTTON_DRAW_FLAT                    ((sal_uInt16)0x0200)
#define BUTTON_DRAW_NOTOPLIGHTBORDER        ((sal_uInt16)0x0400)
#define BUTTON_DRAW_NOBOTTOMSHADOWBORDER    ((sal_uInt16)0x0800)
#define BUTTON_DRAW_NOLEFTLIGHTBORDER       ((sal_uInt16)0x1000)
#define BUTTON_DRAW_NOTEXT                  ((sal_uInt16)0x2000)
#define BUTTON_DRAW_NOIMAGE             ((sal_uInt16)0x4000)
#define BUTTON_DRAW_NODRAW                  ((sal_uInt16)0x8000)

// ------------------
// - DecorationView -
// ------------------

class VCL_DLLPUBLIC DecorationView
{
private:
    OutputDevice*       mpOutDev;

public:
                        DecorationView( OutputDevice* pOutDev )
                            { mpOutDev = pOutDev; }

    void                DrawSymbol( const Rectangle& rRect, SymbolType eType,
                                    const Color& rColor, sal_uInt16 nStyle = 0 );
    void                DrawFrame( const Rectangle& rRect,
                                   const Color& rLeftTopColor,
                                   const Color& rRightBottomColor );
    void                DrawHighlightFrame( const Rectangle& rRect,
                                            sal_uInt16 nStyle = FRAME_HIGHLIGHT_OUT );
    Rectangle           DrawFrame( const Rectangle& rRect, sal_uInt16 nStyle = FRAME_DRAW_OUT );
    Rectangle           DrawButton( const Rectangle& rRect, sal_uInt16 nStyle );
    void                DrawSeparator( const Point& rStart, const Point& rStop, bool bVertical = true );
};

#endif // _SV_DECOVIEW_HXX
