/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salgdi.h,v $
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

#ifndef _SV_SALGDI_H
#define _SV_SALGDI_H

#include <vcl/sv.h>
#include <vcl/sallayout.hxx>
#include <vcl/salgdi.hxx>
#include <vcl/outfont.hxx>
#include <vcl/impfont.hxx>

#include <hash_set>

class ImplOs2FontEntry;

// -----------
// - Defines -
// -----------

// win32 platform specific options. Move them to the PMK file?
#define GCP_USEKERNING    0x0008
#define USE_UNISCRIBE
#define GCP_KERN_HACK
#define GNG_VERT_HACK

// os2 specific physically available font face
class ImplOs2FontData : public ImplFontData
{
public:
                            ImplOs2FontData( PFONTMETRICS,
                                int nFontHeight,
                                BYTE nPitchAndFamily  );
                            ~ImplOs2FontData();

    virtual ImplFontData*   Clone() const;
    virtual ImplFontEntry*  CreateFontInstance( ImplFontSelectData& ) const;
    virtual sal_IntPtr      GetFontId() const;
    void                    SetFontId( sal_IntPtr nId ) { mnId = nId; }
    void                    UpdateFromHPS( HPS );

    bool                    HasChar( sal_uInt32 cChar ) const;

    PFONTMETRICS            GetFontMetrics() const          { return pFontMetric; }
    BYTE                    GetCharSet() const          { return meOs2CharSet; }
    BYTE                    GetPitchAndFamily() const   { return mnPitchAndFamily; }
    bool                    IsGlyphApiDisabled() const  { return mbDisableGlyphApi; }
    bool                    SupportsKorean() const      { return mbHasKoreanRange; }
    bool                    SupportsCJK() const         { return mbHasCJKSupport; }
    bool                    AliasSymbolsHigh() const    { return mbAliasSymbolsHigh; }
    bool                    AliasSymbolsLow() const     { return mbAliasSymbolsLow; }

    ImplFontCharMap*        GetImplFontCharMap();

private:
    sal_IntPtr              mnId;
    bool                    mbDisableGlyphApi;
    bool                    mbHasKoreanRange;
    bool                    mbHasCJKSupport;

    ImplFontCharMap*        mpUnicodeMap;

    // TODO: get rid of the members below needed to work with the Win9x non-unicode API
    BYTE*                   mpFontCharSets;     // all Charsets for the current font (used on W98 for kerning)
    BYTE                    mnFontCharSetCount; // Number of Charsets of the current font; 0 - if not queried
    BYTE                    meOs2CharSet;
    BYTE                    mnPitchAndFamily;
    bool                    mbAliasSymbolsHigh;
    bool                    mbAliasSymbolsLow;
    PFONTMETRICS            pFontMetric;

private:
    void                    ReadCmapTable( HDC );
    void                    ReadOs2Table( HDC );

#ifdef GNG_VERT_HACK
    void                    ReadGsubTable( HDC ) const;

    typedef std::hash_set<int> IntHashSet;
    mutable IntHashSet      maGsubTable;
    mutable bool            mbGsubRead;
public:
    bool                    HasGSUBstitutions( HDC ) const;
    bool                    IsGSUBstituted( sal_Unicode ) const;
#endif // GNG_VERT_HACK

};


// -------------------
// - SalGraphicsData -
// -------------------

class Os2SalGraphics : public SalGraphics
{
public:
    HPS                     mhPS;               // HPS
    HDC                     mhDC;               // HDC
    HWND                    mhWnd;              // HWND
    LONG                    mnHeight;           // Height of frame Window
    ULONG                   mnClipElementCount; // number of clip rects in clip rect array
    RECTL*                  mpClipRectlAry;     // clip rect array
    ULONG                   mnFontMetricCount;  // number of entries in the font list
    PFONTMETRICS            mpFontMetrics;      // cached font list
    LONG                    mnOrientationX;     // X-Font orientation
    LONG                    mnOrientationY;     // Y-Font orientation
    BOOL                    mbLine;             // draw lines
    BOOL                    mbFill;             // fill areas
    BOOL                    mbPrinter;          // is Printer
    BOOL                    mbVirDev;           // is VirDev
    BOOL                    mbWindow;           // is Window
    BOOL                    mbScreen;           // is Screen compatible
    BOOL                    mbXORMode;          // _every_ output with RasterOp XOR
    ULONG                   mhFonts[ MAX_FALLBACK ];        // Font + Fallbacks
    ImplOs2FontData*            mpOs2FontData[ MAX_FALLBACK ];  // pointer to the most recent font face
    ImplOs2FontEntry*           mpOs2FontEntry[ MAX_FALLBACK ]; // pointer to the most recent font instance
    ULONG                   mhDefFont;          // DefaultFont
    float                   mfFontScale;            // allows metrics emulation of huge font sizes
    BOOL                    mbFontKernInit;     // FALSE: FontKerns must be queried
    KERNINGPAIRS*               mpFontKernPairs;    // Kerning Pairs of the current Font
    ULONG                   mnFontKernPairCount;// Number of Kerning Pairs of the current Font

    USHORT                  ImplDoSetFont( ImplFontSelectData* i_pFont, float& o_rFontScale, int );

public:
    Os2SalGraphics();
    virtual ~Os2SalGraphics();

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

#if 0
    // native widget rendering methods that require mirroring
    virtual BOOL        hitTestNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                              const Point& aPos, SalControlHandle& rControlHandle, BOOL& rIsInside );
    virtual BOOL        drawNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle,
                                           rtl::OUString aCaption );
    virtual BOOL        drawNativeControlText( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               SalControlHandle& rControlHandle, rtl::OUString aCaption );
    virtual BOOL        getNativeControlRegion( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, SalControlHandle& rControlHandle, rtl::OUString aCaption,
                                                Region &rNativeBoundingRegion, Region &rNativeContentRegion );
#endif

    virtual bool        drawAlphaBitmap( const SalTwoRect&,
                                         const SalBitmap& rSourceBitmap,
                                         const SalBitmap& rAlphaBitmap );
    virtual bool        drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency );

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
    virtual long            GetGraphicsWidth() const;

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
    // get the repertoire of the current font
    virtual ImplFontCharMap* GetImplFontCharMap() const;
    // graphics must fill supplied font list
    virtual void            GetDevFontList( ImplDevFontList* );
    // graphics should call ImplAddDevFontSubstitute on supplied
    // OutputDevice for all its device specific preferred font substitutions
    virtual void            GetDevFontSubstList( OutputDevice* );
    virtual bool            AddTempDevFont( ImplDevFontList*, const String& rFileURL, const String& rFontName );
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
    virtual const Ucs2SIntMap* GetFontEncodingVector( const ImplFontData*, const Ucs2OStrMap** ppNonEncoded );

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pWidths: the widths of all glyphs from char code 0 to 255
    //                      pWidths MUST support at least 256 members;
    //             rInfo: additional outgoing information
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void* GetEmbedFontData( const ImplFontData*,
                                          const sal_Ucs* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    // frees the font data again
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen );

    virtual void            GetGlyphWidths( ImplFontData* pFont,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc );

    virtual BOOL                    GetGlyphBoundRect( long nIndex, Rectangle& );
    virtual BOOL                    GetGlyphOutline( long nIndex, ::basegfx::B2DPolyPolygon& );

    virtual SalLayout*              GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void                     DrawServerFontLayout( const ServerFontLayout& );
    virtual bool            supportsOperation( OutDevSupportType ) const;

    // Query the platform layer for control support
    virtual BOOL IsNativeControlSupported( ControlType nType, ControlPart nPart );
};

// Init/Deinit Graphics
void ImplSalInitGraphics( Os2SalGraphics* mpData );
void ImplSalDeInitGraphics( Os2SalGraphics* mpData );

// -----------
// - Defines -
// -----------

#define RGBCOLOR(r,g,b)     ((ULONG)(((BYTE)(b)|((USHORT)(g)<<8))|(((ULONG)(BYTE)(r))<<16)))
#define TY( y )             (mnHeight-(y)-1)

// offset for lcid field, used for fallback font selection
#define LCID_BASE                       100

// -----------
// - Inlines -
// -----------

// #102411# Win's GCP mishandles kerning => we need to do it ourselves
// SalGraphicsData::mpFontKernPairs is sorted by
inline bool ImplCmpKernData( const KERNINGPAIRS& a, const KERNINGPAIRS& b )
{
    if( a.sFirstChar < b.sFirstChar )
        return true;
    if( a.sFirstChar > b.sFirstChar )
        return false;
    return (a.sSecondChar < b.sSecondChar);
}

// called extremely often from just one spot => inline
inline bool ImplOs2FontData::HasChar( sal_uInt32 cChar ) const
{
    if( mpUnicodeMap->HasChar( cChar ) )
        return true;
    // second chance to allow symbol aliasing
    if( mbAliasSymbolsLow && ((cChar-0xF000) <= 0xFF) )
        cChar -= 0xF000;
    else if( mbAliasSymbolsHigh && (cChar <= 0xFF) )
        cChar += 0xF000;
    return mpUnicodeMap->HasChar( cChar );
}

#endif // _SV_SALGDI_H
