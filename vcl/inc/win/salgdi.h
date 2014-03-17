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

#ifndef INCLUDED_VCL_INC_WIN_SALGDI_H
#define INCLUDED_VCL_INC_WIN_SALGDI_H

#include <sallayout.hxx>
#include <salgdi.hxx>
#include <outfont.hxx>
#include <impfont.hxx>
#include <vcl/fontcapabilities.hxx>

#include "boost/scoped_ptr.hpp"
#include <boost/unordered_set.hpp>

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#  include <graphite_static.hxx>
#  include <graphite2/Font.h>
#endif

class FontSelectPattern;
class ImplWinFontEntry;
class ImplFontAttrCache;

#define RGB_TO_PALRGB(nRGB)         ((nRGB)|0x02000000)
#define PALRGB_TO_RGB(nPalRGB)      ((nPalRGB)&0x00ffffff)

#if ENABLE_GRAPHITE
class RawFontData;
class GrFontData
{
public:
    GrFontData(HDC hDC);
    ~GrFontData();
    const void * getTable(unsigned int name, size_t *len) const;
    const gr_face * getFace() const { return mpFace; }
    void AddReference() { ++mnRefCount; }
    void DeReference() { if (--mnRefCount == 0) delete this; }
private:
    GrFontData(GrFontData &) {};
    HDC mhDC;
    mutable std::vector<RawFontData*> mvData;
    gr_face * mpFace;
    unsigned int mnRefCount;
};
#endif

// win32 specific physically available font face
class ImplWinFontData : public PhysicalFontFace
{
public:
    explicit                ImplWinFontData( const ImplDevFontAttributes&,
                                int nFontHeight, BYTE eWinCharSet,
                                BYTE nPitchAndFamily  );
    virtual                 ~ImplWinFontData();

    virtual PhysicalFontFace*   Clone() const;
    virtual ImplFontEntry*  CreateFontInstance( FontSelectPattern& ) const;
    virtual sal_IntPtr      GetFontId() const;
    void                    SetFontId( sal_IntPtr nId ) { mnId = nId; }
    void                    UpdateFromHDC( HDC ) const;

    bool                    HasChar( sal_uInt32 cChar ) const;

    BYTE                GetCharSet() const          { return meWinCharSet; }
    BYTE                GetPitchAndFamily() const   { return mnPitchAndFamily; }
    bool                    IsGlyphApiDisabled() const  { return mbDisableGlyphApi; }
    bool                    SupportsKorean() const      { return mbHasKoreanRange; }
    bool                    SupportsCJK() const         { return mbHasCJKSupport; }
    bool                    SupportsArabic() const      { return mbHasArabicSupport; }
    bool                    AliasSymbolsHigh() const    { return mbAliasSymbolsHigh; }
    bool                    AliasSymbolsLow() const     { return mbAliasSymbolsLow; }
#if ENABLE_GRAPHITE
    bool                    SupportsGraphite() const    { return mbHasGraphiteSupport; }
    const gr_face*          GraphiteFace() const;
#endif

    const ImplFontCharMap*  GetImplFontCharMap() const;
    bool GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const;
    const Ucs2SIntMap* GetEncodingVector() const { return mpEncodingVector; }
    void SetEncodingVector( const Ucs2SIntMap* pNewVec ) const
    {
        if( mpEncodingVector )
            delete mpEncodingVector;
        mpEncodingVector = pNewVec;
    }
private:
    sal_IntPtr              mnId;

    // some members that are initalized lazily when the font gets selected into a HDC
    mutable bool                    mbDisableGlyphApi;
    mutable bool                    mbHasKoreanRange;
    mutable bool                    mbHasCJKSupport;
#if ENABLE_GRAPHITE
    mutable GrFontData*             mpGraphiteData;
    mutable bool                    mbHasGraphiteSupport;
#endif
    mutable bool                    mbHasArabicSupport;
    mutable bool                    mbFontCapabilitiesRead;
    mutable ImplFontCharMap*        mpUnicodeMap;
    mutable const Ucs2SIntMap*      mpEncodingVector;
    mutable vcl::FontCapabilities   maFontCapabilities;

    // TODO: get rid of the members below needed to work with the Win9x non-unicode API
    BYTE*                   mpFontCharSets;     // all Charsets for the current font (used on W98 for kerning)
    BYTE                    mnFontCharSetCount; // Number of Charsets of the current font; 0 - if not queried
    BYTE                meWinCharSet;
    BYTE                mnPitchAndFamily;
    bool                    mbAliasSymbolsHigh;
    bool                    mbAliasSymbolsLow;
private:
    void                    ReadCmapTable( HDC ) const;
    void                    GetFontCapabilities( HDC hDC ) const;

    void                    ReadGsubTable( HDC ) const;

    typedef boost::unordered_set<sal_UCS4> UcsHashSet;
    mutable UcsHashSet      maGsubTable;
    mutable bool            mbGsubRead;
public:
    bool                    HasGSUBstitutions( HDC ) const;
    bool                    IsGSUBstituted( sal_UCS4 ) const;
};

class WinSalGraphics : public SalGraphics
{
private:
    HDC                     mhLocalDC;              // HDC

public:
    HDC getHDC() const { return mhLocalDC; }
    void setHDC(HDC aNew) { mhLocalDC = aNew; }

public:
    HWND                    mhWnd;              // Window-Handle, when Window-Graphics
    HFONT                   mhFonts[ MAX_FALLBACK ];        // Font + Fallbacks
    const ImplWinFontData*  mpWinFontData[ MAX_FALLBACK ];  // pointer to the most recent font face
    ImplWinFontEntry*       mpWinFontEntry[ MAX_FALLBACK ]; // pointer to the most recent font instance
    float                   mfFontScale[ MAX_FALLBACK ];        // allows metrics emulation of huge font sizes
    float                   mfCurrentFontScale;
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
    LOGFONTA*               mpLogFont;          // LOG-Font which is currently selected (only W9x)
    ImplFontAttrCache*      mpFontAttrCache;    // Cache font attributes from files in so/share/fonts
    BYTE*                   mpFontCharSets;     // All Charsets for the current font
    BYTE                    mnFontCharSetCount; // Number of Charsets of the current font; 0 - if not queried
    bool                    mbFontKernInit;     // FALSE: FontKerns must be queried
    KERNINGPAIR*            mpFontKernPairs;    // Kerning Pairs of the current Font
    sal_uIntPtr                 mnFontKernPairCount;// Number of Kerning Pairs of the current Font
    int                     mnPenWidth;         // Linienbreite

    /// bitfield
    bool                    mbStockPen : 1;         // is Pen a stockpen
    bool                    mbStockBrush : 1;       // is Brush a stcokbrush
    bool                    mbPen : 1;              // is Pen (FALSE == NULL_PEN)
    bool                    mbBrush : 1;            // is Brush (FALSE == NULL_BRUSH)
    bool                    mbPrinter : 1;          // is Printer
    bool                    mbVirDev : 1;           // is VirDev
    bool                    mbWindow : 1;           // is Window
    bool                    mbScreen : 1;           // is Screen compatible
    bool                    mbXORMode : 1;          // _every_ output with RasterOp XOR

    // remember RGB values for SetLineColor/SetFillColor
    SalColor                maLineColor;
    SalColor                maFillColor;

    HFONT                   ImplDoSetFont( FontSelectPattern* i_pFont, float& o_rFontScale, HFONT& o_rOldFont );

public:
    explicit WinSalGraphics();
    virtual ~WinSalGraphics();

protected:
    virtual bool        setClipRegion( const Region& );
    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void        drawPixel( long nX, long nY );
    virtual void        drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void        drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void        drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void        drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry );
    virtual void        drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry );
    virtual void        drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry );
    virtual bool        drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency );
    virtual bool        drawPolyLine(
        const ::basegfx::B2DPolygon&,
        double fTransparency,
        const ::basegfx::B2DVector& rLineWidth,
        basegfx::B2DLineJoin,
        com::sun::star::drawing::LineCap);
    virtual bool    drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry );
    virtual bool    drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry );
    virtual bool    drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints, const SalPoint* const* pPtAry, const BYTE* const* pFlgAry );

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void        copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                                  long nSrcHeight, sal_uInt16 nFlags );

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void        copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics );
    virtual void        drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap );
    virtual void        drawBitmap( const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    SalColor nTransparentColor );
    virtual void        drawBitmap( const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap );
    virtual void        drawMask( const SalTwoRect& rPosAry,
                                  const SalBitmap& rSalBitmap,
                                  SalColor nMaskColor );

    virtual SalBitmap*  getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor    getPixel( long nX, long nY );

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void        invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags);
    virtual void        invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual bool        drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uIntPtr nSize );

    // native widget rendering methods that require mirroring
    virtual bool        hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                              const Point& aPos, bool& rIsInside ) SAL_OVERRIDE;
    virtual bool        drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue,
                                           const OUString& aCaption );
    virtual bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const OUString& aCaption,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion );

    virtual bool        drawAlphaBitmap( const SalTwoRect&,
                                         const SalBitmap& rSourceBitmap,
                                         const SalBitmap& rAlphaBitmap );
    virtual bool drawTransformedBitmap(
        const basegfx::B2DPoint& rNull,
        const basegfx::B2DPoint& rX,
        const basegfx::B2DPoint& rY,
        const SalBitmap& rSourceBitmap,
        const SalBitmap* pAlphaBitmap);
    virtual bool        drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency );

private:
    // local helpers
    bool tryDrawBitmapGdiPlus(const SalTwoRect& rTR, const SalBitmap& rSrcBitmap);

    // get kernign pairs of the current font
    sal_uLong               GetKernPairs();

public:
    // public SalGraphics methods, the interface to the independent vcl part

    // get device resolution
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    // get the depth of the device
    virtual sal_uInt16          GetBitCount() const;
    // get the width of the device
    virtual long            GetGraphicsWidth() const;

    // set the clip region to empty
    virtual void            ResetClipRegion();

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
    virtual void            SetXORMode( bool bSet, bool );
    // set line color for raster operations
    virtual void            SetROPLineColor( SalROPColor nROPColor );
    // set fill color for raster operations
    virtual void            SetROPFillColor( SalROPColor nROPColor );
    // set the text color to a specific color
    virtual void            SetTextColor( SalColor nSalColor );
    // set the font
    virtual sal_uInt16         SetFont( FontSelectPattern*, int nFallbackLevel );
    // get the current font's metrics
    virtual void            GetFontMetric( ImplFontMetricData*, int nFallbackLevel );
    // get the repertoire of the current font
    virtual const ImplFontCharMap* GetImplFontCharMap() const;
    // get the layout capabilities of the current font
    virtual bool GetImplFontCapabilities(vcl::FontCapabilities &rGetFontCapabilities) const;
    // graphics must fill supplied font list
    virtual void            GetDevFontList( PhysicalFontCollection* );
    // graphics must drop any cached font info
    virtual void ClearDevFontCache();
    virtual bool            AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName );
    // CreateFontSubset: a method to get a subset of glyhps of a font
    // inside a new valid font file
    // returns TRUE if creation of subset was successful
    // parameters: rToFile: contains a osl file URL to write the subset to
    //             pFont: describes from which font to create a subset
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             pWidths: the advance widths of the correspoding glyphs (in PS font units)
    //             nGlyphs: the number of glyphs
    //             rInfo: additional outgoing information
    // implementation note: encoding 0 with glyph id 0 should be added implicitly
    // as "undefined character"
    virtual bool            CreateFontSubset( const OUString& rToFile,
                                              const PhysicalFontFace*,
                                              sal_GlyphId* pGlyphIDs,
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
    virtual const Ucs2SIntMap* GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded );

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pWidths: the widths of all glyphs from char code 0 to 255
    //                      pWidths MUST support at least 256 members;
    //             rInfo: additional outgoing information
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void* GetEmbedFontData( const PhysicalFontFace*,
                                          const sal_Ucs* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    // frees the font data again
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen );
    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc );
    virtual int             GetMinKashidaWidth();

    virtual bool            GetGlyphBoundRect( sal_GlyphId, Rectangle& );
    virtual bool            GetGlyphOutline( sal_GlyphId, ::basegfx::B2DPolyPolygon& );

    virtual SalLayout*              GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void                     DrawServerFontLayout( const ServerFontLayout& );

    virtual bool            supportsOperation( OutDevSupportType ) const;
    // Query the platform layer for control support
    virtual bool IsNativeControlSupported( ControlType nType, ControlPart nPart );

    virtual SystemGraphicsData GetGraphicsData() const;
    virtual SystemFontData     GetSysFontData( int nFallbacklevel ) const;

    /// Update settings based on the platform values
    static void updateSettingsNative( AllSettings& rSettings );
};

// Init/Deinit Graphics
void    ImplSalInitGraphics( WinSalGraphics* );
void    ImplSalDeInitGraphics( WinSalGraphics* );
void    ImplUpdateSysColorEntries();
int     ImplIsSysColorEntry( SalColor nSalColor );
void    ImplGetLogFontFromFontSelect( HDC, const FontSelectPattern*,
            LOGFONTW&, bool bTestVerticalAvail );

#define MAX_64KSALPOINTS    ((((sal_uInt16)0xFFFF)-8)/sizeof(POINTS))

// #102411# Win's GCP mishandles kerning => we need to do it ourselves
// SalGraphicsData::mpFontKernPairs is sorted by
inline bool ImplCmpKernData( const KERNINGPAIR& a, const KERNINGPAIR& b )
{
    if( a.wFirst < b.wFirst )
        return true;
    if( a.wFirst > b.wFirst )
        return false;
    return (a.wSecond < b.wSecond);
}

// called extremely often from just one spot => inline
inline bool ImplWinFontData::HasChar( sal_uInt32 cChar ) const
{
    if( mpUnicodeMap->HasChar( cChar ) )
        return true;
    // second chance to allow symbol aliasing
    if( mbAliasSymbolsLow && ((cChar-0xF000) <= 0xFF) )
        cChar -= 0xF000;
    else if( mbAliasSymbolsHigh && (cChar <= 0xFF) )
        cChar += 0xF000;
    else
        return false;
    return mpUnicodeMap->HasChar( cChar );
}

#endif // INCLUDED_VCL_INC_WIN_SALGDI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
