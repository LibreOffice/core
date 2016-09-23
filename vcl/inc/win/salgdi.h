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

#include "sallayout.hxx"
#include "salgeom.hxx"
#include "salgdi.hxx"
#include "fontinstance.hxx"
#include "fontattributes.hxx"
#include "PhysicalFontFace.hxx"
#include "impfont.hxx"
#include <vcl/fontcapabilities.hxx>
#include <vcl/fontcharmap.hxx>

#include <memory>
#include <unordered_set>

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#  include <graphite_static.hxx>
#  include <graphite2/Font.h>
#endif
#ifndef INCLUDED_PRE_POST_WIN_H
#define INCLUDED_PRE_POST_WIN_H
#  include "prewin.h"
#  include "postwin.h"
#endif

class FontSelectPattern;
class WinFontInstance;
class ImplFontAttrCache;
class OpenGLTexture;
class PhysicalFontCollection;
class SalGraphicsImpl;
class WinOpenGLSalGraphicsImpl;
class ImplFontMetricData;
class CommonSalLayout;

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
class WinFontFace : public PhysicalFontFace
{
public:
    explicit                WinFontFace( const FontAttributes&,
                                int nFontHeight, BYTE eWinCharSet,
                                BYTE nPitchAndFamily  );
    virtual                 ~WinFontFace();

    virtual PhysicalFontFace* Clone() const override;
    virtual LogicalFontInstance* CreateFontInstance( FontSelectPattern& ) const override;
    virtual sal_IntPtr      GetFontId() const override;
    void                    SetFontId( sal_IntPtr nId ) { mnId = nId; }
    void                    UpdateFromHDC( HDC ) const;

    bool                    HasChar( sal_uInt32 cChar ) const;

    BYTE                    GetCharSet() const          { return meWinCharSet; }
    BYTE                    GetPitchAndFamily() const   { return mnPitchAndFamily; }
    bool                    SupportsCJK() const         { return mbHasCJKSupport; }
    bool                    SupportsArabic() const      { return mbHasArabicSupport; }
#if ENABLE_GRAPHITE
    bool                    SupportsGraphite() const    { return mbHasGraphiteSupport; }
    const gr_face*          GraphiteFace() const;
#endif

    FontCharMapRef          GetFontCharMap() const;
    bool GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const;
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
    mutable bool                    mbHasCJKSupport;
#if ENABLE_GRAPHITE
    mutable GrFontData*             mpGraphiteData;
    mutable bool                    mbHasGraphiteSupport;
#endif
    mutable bool                    mbHasArabicSupport;
    mutable bool                    mbFontCapabilitiesRead;
    mutable FontCharMapRef          mxUnicodeMap;
    mutable const Ucs2SIntMap*      mpEncodingVector;
    mutable vcl::FontCapabilities   maFontCapabilities;

    BYTE                    meWinCharSet;
    BYTE                    mnPitchAndFamily;
    bool                    mbAliasSymbolsHigh;
    bool                    mbAliasSymbolsLow;
private:
    void                    ReadCmapTable( HDC ) const;
    void                    GetFontCapabilities( HDC hDC ) const;

    void                    ReadGsubTable( HDC ) const;

    mutable std::unordered_set<sal_UCS4>      maGsubTable;
    mutable bool            mbGsubRead;
public:
    bool                    HasGSUBstitutions( HDC ) const;
    bool                    IsGSUBstituted( sal_UCS4 ) const;
    static int              GetTable( const char pTagName[5], const unsigned char*&, HDC );
};

/** Class that creates (and destroys) a compatible Device Context.

This is to be used for GDI drawing into a DIB that we later use as a texture for OpenGL drawing.
*/
class OpenGLCompatibleDC
{
private:
    /// The compatible DC that we create for our purposes.
    HDC mhCompatibleDC;

    /// DIBSection that we use for the GDI drawing, and later obtain.
    HBITMAP mhBitmap;

    /// Return the previous bitmap to undo the SelectObject.
    HBITMAP mhOrigBitmap;

    /// DIBSection data.
    sal_uInt32 *mpData;

    /// Mapping between the GDI position and OpenGL, to use for OpenGL drawing.
    SalTwoRect maRects;

    /// The OpenGL-based SalGraphicsImpl where we will draw.  If null, we ignore the drawing, it means it happened directly to the DC...
    WinOpenGLSalGraphicsImpl *mpImpl;

public:
    OpenGLCompatibleDC(SalGraphics &rGraphics, int x, int y, int width, int height);
    ~OpenGLCompatibleDC();

    HDC getCompatibleHDC() { return mhCompatibleDC; }

    SalTwoRect getTwoRect() { return maRects; }

    Size getBitmapSize() { return Size(maRects.mnSrcWidth, maRects.mnSrcHeight); }

    /// Reset the DC with the defined color.
    void fill(sal_uInt32 color);

    /// Obtain the texture; the caller must delete it after use.
    OpenGLTexture* getTexture();

    /// Copy bitmap data to the texture. Texutre must be initialized and the correct size to hold the bitmap.
    bool copyToTexture(OpenGLTexture& aTexture);
};

class WinSalGraphics : public SalGraphics
{
    friend class WinSalGraphicsImpl;
    friend class WinOpenGLSalGraphicsImpl;
    friend class ScopedFont;
    friend class OpenGLCompatibleDC;
    friend class WinLayout;
    friend class SimpleWinLayout;
    friend class UniscribeLayout;

protected:
    std::unique_ptr<SalGraphicsImpl> mpImpl;

private:
    HDC                     mhLocalDC;              // HDC
    bool                    mbPrinter : 1;          // is Printer
    bool                    mbVirDev : 1;           // is VirDev
    bool                    mbWindow : 1;           // is Window
    bool                    mbScreen : 1;           // is Screen compatible
    HWND                    mhWnd;              // Window-Handle, when Window-Graphics

    HFONT                   mhFonts[ MAX_FALLBACK ];        // Font + Fallbacks
    const WinFontFace*  mpWinFontData[ MAX_FALLBACK ];  // pointer to the most recent font face
    WinFontInstance*       mpWinFontEntry[ MAX_FALLBACK ]; // pointer to the most recent font instance
    float                   mfFontScale[ MAX_FALLBACK ];        // allows metrics emulation of huge font sizes
    float                   mfCurrentFontScale;
    HRGN                    mhRegion;           // vcl::Region Handle
    HPEN                    mhDefPen;           // DefaultPen
    HBRUSH                  mhDefBrush;         // DefaultBrush
    HFONT                   mhDefFont;          // DefaultFont
    HPALETTE                mhDefPal;           // DefaultPalette
    COLORREF                mnTextColor;        // TextColor
    RGNDATA*                mpClipRgnData;      // ClipRegion-Data
    RGNDATA*                mpStdClipRgnData;   // Cache Standard-ClipRegion-Data
    ImplFontAttrCache*      mpFontAttrCache;    // Cache font attributes from files in so/share/fonts
    bool                    mbFontKernInit;     // FALSE: FontKerns must be queried
    KERNINGPAIR*            mpFontKernPairs;    // Kerning Pairs of the current Font
    sal_uIntPtr             mnFontKernPairCount;// Number of Kerning Pairs of the current Font
    int                     mnPenWidth;         // Linienbreite

    LogicalFontInstance* GetWinFontEntry(int nFallbackLevel);

public:
    HDC getHDC() const { return mhLocalDC; }
    void setHDC(HDC aNew) { mhLocalDC = aNew; }

    HPALETTE getDefPal() const;
    void setDefPal(HPALETTE hDefPal);

    HRGN getRegion() const;

    void InitGraphics();
    void DeInitGraphics();

    enum Type
    {
        PRINTER,
        VIRTUAL_DEVICE,
        WINDOW,
        SCREEN
    };

public:

    HWND gethWnd();

    HFONT                   ImplDoSetFont( FontSelectPattern* i_pFont, float& o_rFontScale, HFONT& o_rOldFont );

public:
    explicit WinSalGraphics(WinSalGraphics::Type eType, bool bScreen, HWND hWnd,
                            SalGeometryProvider *pProvider);
    virtual ~WinSalGraphics();

    SalGraphicsImpl* GetImpl() const override;
    bool isPrinter() const;
    bool isVirtualDevice() const;
    bool isWindow() const;
    bool isScreen() const;

    void setHWND(HWND hWnd);

protected:
    virtual bool        setClipRegion( const vcl::Region& ) override;
    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void        drawPixel( long nX, long nY ) override;
    virtual void        drawPixel( long nX, long nY, SalColor nSalColor ) override;
    virtual void        drawLine( long nX1, long nY1, long nX2, long nY2 ) override;
    virtual void        drawRect( long nX, long nY, long nWidth, long nHeight ) override;
    virtual void        drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
    virtual void        drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
    virtual void        drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry ) override;
    virtual bool        drawPolyPolygon( const basegfx::B2DPolyPolygon&, double fTransparency ) override;
    virtual bool        drawPolyLine(
        const basegfx::B2DPolygon&,
        double fTransparency,
        const basegfx::B2DVector& rLineWidth,
        basegfx::B2DLineJoin,
        css::drawing::LineCap,
        double fMiterMinimumAngle) override;
    virtual bool        drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry ) override;
    virtual bool        drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry ) override;
    virtual bool        drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints, const SalPoint* const* pPtAry, const BYTE* const* pFlgAry ) override;
    virtual bool        drawGradient( const tools::PolyPolygon&, const Gradient& ) override { return false; };

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void        copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                                  long nSrcHeight, bool bWindowInvalidate ) override;

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void        copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) override;
    virtual void        drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap ) override;
    virtual void        drawBitmap( const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap ) override;
    virtual void        drawMask( const SalTwoRect& rPosAry,
                                  const SalBitmap& rSalBitmap,
                                  SalColor nMaskColor ) override;

    virtual SalBitmap*  getBitmap( long nX, long nY, long nWidth, long nHeight ) override;
    virtual SalColor    getPixel( long nX, long nY ) override;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void        invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags) override;
    virtual void        invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) override;

    virtual bool        drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uIntPtr nSize ) override;

    // native widget rendering methods that require mirroring
    virtual bool        hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                              const Point& aPos, bool& rIsInside ) override;
    virtual bool        drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue,
                                           const OUString& aCaption ) override;
    virtual bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const OUString& aCaption,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion ) override;

    virtual bool        blendBitmap( const SalTwoRect&,
                                     const SalBitmap& rBitmap ) override;

    virtual bool        blendAlphaBitmap( const SalTwoRect&,
                                          const SalBitmap& rSrcBitmap,
                                          const SalBitmap& rMaskBitmap,
                                          const SalBitmap& rAlphaBitmap ) override;

    virtual bool        drawAlphaBitmap( const SalTwoRect&,
                                         const SalBitmap& rSourceBitmap,
                                         const SalBitmap& rAlphaBitmap ) override;
    virtual bool       drawTransformedBitmap(
                           const basegfx::B2DPoint& rNull,
                           const basegfx::B2DPoint& rX,
                           const basegfx::B2DPoint& rY,
                           const SalBitmap& rSourceBitmap,
                           const SalBitmap* pAlphaBitmap) override;
    virtual bool       drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency ) override;

private:
    // local helpers

    // get kernign pairs of the current font
    sal_uLong               GetKernPairs();

public:
    // public SalGraphics methods, the interface to the independent vcl part

    // get device resolution
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;
    // get the depth of the device
    virtual sal_uInt16          GetBitCount() const override;
    // get the width of the device
    virtual long            GetGraphicsWidth() const override;

    // set the clip region to empty
    virtual void            ResetClipRegion() override;

    // set the line color to transparent (= don't draw lines)
    virtual void            SetLineColor() override;
    // set the line color to a specific color
    virtual void            SetLineColor( SalColor nSalColor ) override;
    // set the fill color to transparent (= don't fill)
    virtual void            SetFillColor() override;
    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void            SetFillColor( SalColor nSalColor ) override;
    // enable/disable XOR drawing
    virtual void            SetXORMode( bool bSet ) override;
    // set line color for raster operations
    virtual void            SetROPLineColor( SalROPColor nROPColor ) override;
    // set fill color for raster operations
    virtual void            SetROPFillColor( SalROPColor nROPColor ) override;
    // set the text color to a specific color
    virtual void            SetTextColor( SalColor nSalColor ) override;
    // set the font
    virtual void            SetFont( FontSelectPattern*, int nFallbackLevel ) override;
    // get the current font's metrics
    virtual void            GetFontMetric( ImplFontMetricDataRef&, int nFallbackLevel ) override;
    // get the repertoire of the current font
    virtual const FontCharMapRef GetFontCharMap() const override;
    // get the layout capabilities of the current font
    virtual bool GetFontCapabilities(vcl::FontCapabilities &rGetFontCapabilities) const override;
    // graphics must fill supplied font list
    virtual void            GetDevFontList( PhysicalFontCollection* ) override;
    // graphics must drop any cached font info
    virtual void            ClearDevFontCache() override;
    virtual bool            AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) override;
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
                                              const sal_GlyphId* pGlyphIDs,
                                              const sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo // out parameter
                                              ) override;

    // GetFontEncodingVector: a method to get the encoding map Unicode
    // to font encoded character; this is only used for type1 fonts and
    // may return NULL in case of unknown encoding vector
    // if ppNonEncoded is set and non encoded characters (that is type1
    // glyphs with only a name) exist it is set to the corresponding
    // map for non encoded glyphs; the encoding vector contains -1
    // as encoding for these cases
    virtual const Ucs2SIntMap* GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded, std::set<sal_Unicode> const** ) override;

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
                                          size_t nLen,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen ) override;
    // frees the font data again
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen ) override;
    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            std::vector< sal_Int32 >& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) override;
            int             GetMinKashidaWidth();

    virtual bool            GetGlyphBoundRect( sal_GlyphId, Rectangle& ) override;
    virtual bool            GetGlyphOutline( sal_GlyphId, basegfx::B2DPolyPolygon& ) override;

    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) override;
    virtual void            DrawSalLayout( const CommonSalLayout& ) override;
    virtual void            DrawServerFontLayout( const GenericSalLayout&, const ServerFont& ) {};

    virtual bool            supportsOperation( OutDevSupportType ) const override;
    // Query the platform layer for control support
    virtual bool            IsNativeControlSupported( ControlType nType, ControlPart nPart ) override;

    virtual SystemGraphicsData GetGraphicsData() const override;

    /// Update settings based on the platform values
    static void updateSettingsNative( AllSettings& rSettings );
};

// Init/Deinit Graphics
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
inline bool WinFontFace::HasChar( sal_uInt32 cChar ) const
{
    if( mxUnicodeMap->HasChar( cChar ) )
        return true;
    // second chance to allow symbol aliasing
    if( mbAliasSymbolsLow && ((cChar-0xF000) <= 0xFF) )
        cChar -= 0xF000;
    else if( mbAliasSymbolsHigh && (cChar <= 0xFF) )
        cChar += 0xF000;
    else
        return false;
    return mxUnicodeMap->HasChar( cChar );
}

#endif // INCLUDED_VCL_INC_WIN_SALGDI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
