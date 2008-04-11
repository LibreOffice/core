/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: decoview.hxx,v $
 * $Revision: 1.4 $
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
class Color;
class OutputDevice;

// ------------------------
// - DecorationView-Types -
// ------------------------

// Flags for DrawSymbol()
#define SYMBOL_DRAW_MONO                    ((USHORT)0x0001)
#define SYMBOL_DRAW_DISABLE                 ((USHORT)0x0002)

// Flags for DrawFrame()
#define FRAME_DRAW_IN                       ((USHORT)0x0001)
#define FRAME_DRAW_OUT                      ((USHORT)0x0002)
#define FRAME_DRAW_GROUP                    ((USHORT)0x0003)
#define FRAME_DRAW_DOUBLEIN                 ((USHORT)0x0004)
#define FRAME_DRAW_DOUBLEOUT                ((USHORT)0x0005)
#define FRAME_DRAW_MENU                     ((USHORT)0x0010)
#define FRAME_DRAW_WINDOWBORDER             ((USHORT)0x0020)
#define FRAME_DRAW_BORDERWINDOWBORDER       ((USHORT)0x0040)
#define FRAME_DRAW_MONO                     ((USHORT)0x1000)
#define FRAME_DRAW_NODRAW                   ((USHORT)0x8000)
#define FRAME_DRAW_STYLE                    ((USHORT)0x000F)

// Flags for DrawHighlightFrame()
#define FRAME_HIGHLIGHT_IN                  ((USHORT)0x0001)
#define FRAME_HIGHLIGHT_OUT                 ((USHORT)0x0002)
#define FRAME_HIGHLIGHT_TESTBACKGROUND      ((USHORT)0x4000)
#define FRAME_HIGHLIGHT_STYLE               ((USHORT)0x000F)

// Flags for DrawButton()
#define BUTTON_DRAW_DEFAULT                 ((USHORT)0x0001)
#define BUTTON_DRAW_NOLIGHTBORDER           ((USHORT)0x0002)
#define BUTTON_DRAW_PRESSED                 ((USHORT)0x0004)
#define BUTTON_DRAW_CHECKED                 ((USHORT)0x0008)
#define BUTTON_DRAW_DONTKNOW                ((USHORT)0x0010)
#define BUTTON_DRAW_MONO                    ((USHORT)0x0020)
#define BUTTON_DRAW_NOFILL                  ((USHORT)0x0040)
#define BUTTON_DRAW_DISABLED                ((USHORT)0x0080)
#define BUTTON_DRAW_HIGHLIGHT               ((USHORT)0x0100)
#define BUTTON_DRAW_FLAT                    ((USHORT)0x0200)
#define BUTTON_DRAW_NOTOPLIGHTBORDER        ((USHORT)0x0400)
#define BUTTON_DRAW_NOBOTTOMSHADOWBORDER    ((USHORT)0x0800)
#define BUTTON_DRAW_NOLEFTLIGHTBORDER       ((USHORT)0x1000)
#define BUTTON_DRAW_NOTEXT                  ((USHORT)0x2000)
#define BUTTON_DRAW_NOIMAGE             ((USHORT)0x4000)
#define BUTTON_DRAW_NODRAW                  ((USHORT)0x8000)

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
                                    const Color& rColor, USHORT nStyle = 0 );
    void                DrawFrame( const Rectangle& rRect,
                                   const Color& rLeftTopColor,
                                   const Color& rRightBottomColor );
    void                DrawHighlightFrame( const Rectangle& rRect,
                                            USHORT nStyle = FRAME_HIGHLIGHT_OUT );
    Rectangle           DrawFrame( const Rectangle& rRect, USHORT nStyle = FRAME_DRAW_OUT );
    Rectangle           DrawButton( const Rectangle& rRect, USHORT nStyle );
};

#endif // _SV_DECOVIEW_HXX
