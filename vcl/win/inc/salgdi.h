/*************************************************************************
 *
 *  $RCSfile: salgdi.h,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:48:47 $
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
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
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

    virtual void flush( int nMinLevel ) = 0;
};

// -------------------
// - SalGraphicsData -
// -------------------

class WinSalGraphics : public SalGraphics
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
public:
    WinSalGraphics();
    virtual ~WinSalGraphics();

protected:
    virtual BOOL        unionClipRegion( long nX, long nY, long nWidth, long nHeight );
    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void        drawPixel( long nX, long nY );
    virtual void        drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void        drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void        drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void        drawPolyLine( ULONG nPoints, const SalPoint* pPtAry );
    virtual void        drawPolygon( ULONG nPoints, const SalPoint* pPtAry );
    virtual void        drawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry );
    virtual sal_Bool    drawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry );
    virtual sal_Bool    drawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry );
    virtual sal_Bool    drawPolyPolygonBezier( ULONG nPoly, const ULONG* pPoints, const SalPoint* const* pPtAry, const BYTE* const* pFlgAry );

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void        copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                                  long nSrcHeight, USHORT nFlags );

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void        copyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics );
    virtual void        drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap );
    virtual void        drawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap,
                                    SalColor nTransparentColor );
    virtual void        drawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap );
    virtual void        drawMask( const SalTwoRect* pPosAry,
                                  const SalBitmap& rSalBitmap,
                                  SalColor nMaskColor );

    virtual SalBitmap*  getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor    getPixel( long nX, long nY );

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void        invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags);
    virtual void        invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual BOOL        drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize );

public:
    // public SalGraphics methods, the interface to teh independent vcl part

    // get device resolution
    virtual void            GetResolution( long& rDPIX, long& rDPIY );
    // get resolution for fonts (an implementations specific adjustment,
    // ideally would be the same as the Resolution)
    virtual void            GetScreenFontResolution( long& rDPIX, long& rDPIY );
    // get the depth of the device
    virtual USHORT          GetBitCount();
    // get the width of the device
    virtual long            GetGraphicsWidth();

    // set the clip region to empty
    virtual void            ResetClipRegion();
    // begin setting the clip region, add rectangles to the
    // region with the UnionClipRegion call
    virtual void            BeginSetClipRegion( ULONG nCount );
    // all rectangles were added and the clip region should be set now
    virtual void            EndSetClipRegion();

    // set the line color to transparent (= don't draw lines)
    virtual void            SetLineColor();
    // set the line color to a specific color
    virtual void            SetLineColor( SalColor nSalColor );
    // set the fill color to transparent (= don't fill)
    virtual void            SetFillColor();
    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void            SetFillColor( SalColor nSalColor );
    // enable/disable XOR drawing
    virtual void            SetXORMode( BOOL bSet );
    // set line color for raster operations
    virtual void            SetROPLineColor( SalROPColor nROPColor );
    // set fill color for raster operations
    virtual void            SetROPFillColor( SalROPColor nROPColor );
    // set the text color to a specific color
    virtual void            SetTextColor( SalColor nSalColor );
    // set the font
    virtual USHORT         SetFont( ImplFontSelectData*, int nFallbackLevel );
    // get the current font's etrics
    virtual void            GetFontMetric( ImplFontMetricData* );
    // get kernign pairs of the current font
    // return only PairCount if (pKernPairs == NULL)
    virtual ULONG           GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs );
    // get the repertoire of the current font; the code pairs returned
    // contain unicode ranges. if pCodePairs is NULL return only the
    // number of pairs which would be filled
    virtual ULONG           GetFontCodeRanges( sal_uInt32* pCodePairs ) const;
    // graphics must fill supplied font list
    virtual void            GetDevFontList( ImplDevFontList* );
    // graphics should call ImplAddDevFontSubstitute on supplied
    // OutputDevice for all its device specific preferred font substitutions
    virtual void            GetDevFontSubstList( OutputDevice* );
    virtual ImplFontData*   AddTempDevFont( const String& rFileURL, const String& rFontName );
    // CreateFontSubset: a method to get a subset of glyhps of a font
    // inside a new valid font file
    // returns TRUE if creation of subset was successfull
    // parameters: rToFile: contains a osl file URL to write the subset to
    //             pFont: describes from which font to create a subset
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             pWidths: the advance widths of the correspoding glyphs (in PS font units)
    //             nGlyphs: the number of glyphs
    //             rInfo: additional outgoing information
    // implementation note: encoding 0 with glyph id 0 should be added implicitly
    // as "undefined character"
    virtual BOOL            CreateFontSubset( const rtl::OUString& rToFile,
                                              ImplFontData* pFont,
                                              long* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo // out parameter
                                              );

    // GetFontEncodingVector: a method to get the encoding map Unicode
    // to font encoded character; this is only used for type1 fonts and
    // may return NULL in case of unknown encoding vector
    // if ppNonEncoded is set and non encoded characters (that is type1
    // glyphs with only a name) exist it is set to the corresponding
    // map for non encoded glyphs; the encoding vector contains -1
    // as encoding for these cases
    virtual const std::map< sal_Unicode, sal_Int32 >* GetFontEncodingVector( ImplFontData* pFont, const std::map< sal_Unicode, rtl::OString >** ppNonEncoded );

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pWidths: the widths of all glyphs from char code 0 to 255
    //                      pWidths MUST support at least 256 members;
    //             rInfo: additional outgoing information
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void* GetEmbedFontData( ImplFontData* pFont,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    // frees the font data again
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen );

    virtual BOOL                    GetGlyphBoundRect( long nIndex, Rectangle& );
    virtual BOOL                    GetGlyphOutline( long nIndex, PolyPolygon& );

    virtual SalLayout*              GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void                     DrawServerFontLayout( const ServerFontLayout& );
};

// Init/Deinit Graphics
void    ImplSalInitGraphics( WinSalGraphics* mpData );
void    ImplSalDeInitGraphics( WinSalGraphics* mpData );
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
