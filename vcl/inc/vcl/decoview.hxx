/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: decoview.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 11:00:23 $
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

#ifndef _SV_DECOVIEW_HXX
#define _SV_DECOVIEW_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_SYMBOL_HXX
#include <vcl/symbol.hxx>
#endif

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
