/*************************************************************************
 *
 *  $RCSfile: salgdi.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 15:20:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_SALGDI_H
#define _SV_SALGDI_H

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif

#include "boost/scoped_ptr.hpp"

struct ImplFontSelectData;

// -----------
// - Defines -
// -----------

#define RGB_TO_PALRGB(nRGB)         ((nRGB)|0x02000000)
#define PALRGB_TO_RGB(nPalRGB)      ((nPalRGB)&0x00ffffff)

// win32 platform specifics, maybe move pmk file
#define USE_UNISCRIBE
#define GCP_KERN_HACK

// Instances of classes derived from SalLayout might collectively want to cache
// font data; see the mxTextLayoutCache member of SalGraphicsData.  Since
// different derived classes will typically cache different data, this abstract
// class offers only the minimal interface needed from the outside:
class ImplTextLayoutCache
{
public:
    virtual inline ~ImplTextLayoutCache() {};

    virtual void flush() = 0;
};

// -------------------
// - SalGraphicsData -
// -------------------

class SalGraphicsData
{
public:
    HDC                     mhDC;               // HDC
    HWND                    mhWnd;              // Window-Handle, when Window-Graphics
    HFONT                   mhFonts[ MAX_FALLBACK ]; // Font + Fallbacks
    HPEN                    mhPen;              // Pen
    HBRUSH                  mhBrush;            // Brush
    HRGN                    mhRegion;           // Region Handle
    HPEN                    mhDefPen;           // DefaultPen
    HBRUSH                  mhDefBrush;         // DefaultBrush
    HFONT                   mhDefFont;          // DefaultFont
    HPALETTE                mhDefPal;           // DefaultPalette
    COLORREF                mnPenColor;         // PenColor
    COLORREF                mnBrushColor;       // BrushColor
    COLORREF                mnTextColor;        // TextColor
    RGNDATA*                mpClipRgnData;      // ClipRegion-Data
    RGNDATA*                mpStdClipRgnData;   // Cache Standard-ClipRegion-Data
    RECT*                   mpNextClipRect;     // Naechstes ClipRegion-Rect
    BOOL                    mbFirstClipRect;    // Flag for first cliprect to insert
    LOGFONTA*               mpLogFont;          // LOG-Font which is currently selected (only W9x)
    BYTE*                   mpFontCharSets;     // All Charsets for the current font
    BYTE                    mnFontCharSetCount; // Number of Charsets of the current font; 0 - if not queried
    KERNINGPAIR*            mpFontKernPairs;    // Kerning Pairs of the current Font
    ULONG                   mnFontKernPairCount;// Number of Kerning Pairs of the current Font
    BOOL                    mbFontKernInit;     // FALSE: FontKerns must be queried
    int                     mnPenWidth;         // Linienbreite
    BOOL                    mbStockPen;         // is Pen a stockpen
    BOOL                    mbStockBrush;       // is Brush a stcokbrush
    BOOL                    mbPen;              // is Pen (FALSE == NULL_PEN)
    BOOL                    mbBrush;            // is Brush (FALSE == NULL_BRUSH)
    BOOL                    mbPrinter;          // is Printer
    BOOL                    mbVirDev;           // is VirDev
    BOOL                    mbWindow;           // is Window
    BOOL                    mbScreen;           // is Screen compatible
    BOOL                    mbXORMode;          // _every_ output with RasterOp XOR

    // Used collectively by the (derived) SalLayout instances returned by
    // SalGraphics::GetTextLayout, to cache data derived from mhDC's font;
    // flushed whenever a new font is selected into mhDC:
    boost::scoped_ptr< ImplTextLayoutCache > mxTextLayoutCache;
};

// Init/Deinit Graphics
void    ImplSalInitGraphics( SalGraphicsData* mpData );
void    ImplSalDeInitGraphics( SalGraphicsData* mpData );
void    ImplUpdateSysColorEntries();
int     ImplIsSysColorEntry( SalColor nSalColor );
void    ImplGetLogFontFromFontSelect( HDC hDC,
                                      const ImplFontSelectData* pFont,
                                      LOGFONTW& rLogFont,
                                      bool bTestVerticalAvail );

// -----------
// - Defines -
// -----------

#ifdef WIN
#define MAX_64KSALPOINTS    ((((USHORT)0xFFFF)-4)/sizeof(POINT))
#else
#define MAX_64KSALPOINTS    ((((USHORT)0xFFFF)-8)/sizeof(POINTS))
#endif

// -----------
// - Inlines -
// -----------

// #102411# Win's GCP mishandles kerning => we need to do it ourselves
// SalGraphicsData::mpFontKernPairs is sorted by
inline bool ImplCmpKernData( const KERNINGPAIR& a, const KERNINGPAIR& b )
{
    if( a.wFirst < b.wFirst )
        return true;
    if( (a.wFirst == b.wFirst) && (a.wSecond < b.wSecond) )
        return true;
    return false;
}

#endif // _SV_SALGDI_H
