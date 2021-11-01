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
#include <salgeom.hxx>
#include <salgdi.hxx>
#include <fontinstance.hxx>
#include <fontattributes.hxx>
#include <PhysicalFontFace.hxx>
#include <impfont.hxx>
#include <vcl/fontcapabilities.hxx>
#include <vcl/fontcharmap.hxx>

#include <memory>
#include <unordered_set>

#ifndef INCLUDED_PRE_POST_WIN_H
#define INCLUDED_PRE_POST_WIN_H
#  include <prewin.h>
#  include <postwin.h>
#endif

#include <hb-ot.h>
#include <dwrite.h>

class FontSelectPattern;
class WinFontInstance;
class ImplFontAttrCache;
class PhysicalFontCollection;
class SalGraphicsImpl;
class WinSalGraphicsImplBase;
class ImplFontMetricData;

#define RGB_TO_PALRGB(nRGB)         ((nRGB)|0x02000000)
#define PALRGB_TO_RGB(nPalRGB)      ((nPalRGB)&0x00ffffff)

// win32 specific physically available font face
class WinFontFace : public PhysicalFontFace
{
public:
    explicit                WinFontFace( const FontAttributes&,
                                BYTE eWinCharSet,
                                BYTE nPitchAndFamily  );
    virtual                 ~WinFontFace() override;

    virtual rtl::Reference<LogicalFontInstance> CreateFontInstance( const FontSelectPattern& ) const override;
    virtual sal_IntPtr      GetFontId() const override;
    void                    SetFontId( sal_IntPtr nId ) { mnId = nId; }
    void                    UpdateFromHDC( HDC ) const;

    bool                    HasChar( sal_uInt32 cChar ) const;

    BYTE                    GetCharSet() const          { return meWinCharSet; }
    BYTE                    GetPitchAndFamily() const   { return mnPitchAndFamily; }

    FontCharMapRef GetFontCharMap() const override;
    bool GetFontCapabilities(vcl::FontCapabilities&) const override;

private:
    sal_IntPtr              mnId;

    // some members that are initialized lazily when the font gets selected into a HDC
    mutable bool                    mbFontCapabilitiesRead;
    mutable FontCharMapRef          mxUnicodeMap;
    mutable vcl::FontCapabilities   maFontCapabilities;

    BYTE                    meWinCharSet;
    BYTE                    mnPitchAndFamily;
    bool                    mbAliasSymbolsHigh;
    bool                    mbAliasSymbolsLow;

    void                    ReadCmapTable( HDC ) const;
    void                    GetFontCapabilities( HDC hDC ) const;
};

/** Class that creates (and destroys) a compatible Device Context.

This is to be used for GDI drawing into a DIB that we later use for a different
drawing method, such as a texture for OpenGL drawing or surface for Skia drawing.
*/
class CompatibleDC
{
protected:
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

    /// The SalGraphicsImpl where we will draw.  If null, we ignore the drawing, it means it happened directly to the DC...
    WinSalGraphicsImplBase *mpImpl;

    // If 'disable' is true, this class is a simple wrapper for drawing directly. Subclasses should use true.
    CompatibleDC(SalGraphics &rGraphics, int x, int y, int width, int height, bool disable=true);

public:
    static std::unique_ptr< CompatibleDC > create(SalGraphics &rGraphics, int x, int y, int width, int height);

    virtual ~CompatibleDC();

    HDC getCompatibleHDC() { return mhCompatibleDC; }

    SalTwoRect getTwoRect() const { return maRects; }

    tools::Long getBitmapWidth() const { return maRects.mnSrcWidth; }
    tools::Long getBitmapHeight() const { return maRects.mnSrcHeight; }

    /// Reset the DC with the defined color.
    void fill(sal_uInt32 color);
};

class WinSalGraphics : public SalGraphics
{
    friend class WinSalGraphicsImpl;
    friend class ScopedFont;

protected:
    std::unique_ptr<SalGraphicsImpl> mpImpl;

private:
    HDC                     mhLocalDC;              // HDC
    bool                    mbPrinter : 1;          // is Printer
    bool                    mbVirDev : 1;           // is VirDev
    bool                    mbWindow : 1;           // is Window
    bool                    mbScreen : 1;           // is Screen compatible
    HWND                    mhWnd;              // Window-Handle, when Window-Graphics

    rtl::Reference<WinFontInstance>
                            mpWinFontEntry[ MAX_FALLBACK ]; // pointer to the most recent font instance
    HRGN                    mhRegion;           // vcl::Region Handle
    HPEN                    mhDefPen;           // DefaultPen
    HBRUSH                  mhDefBrush;         // DefaultBrush
    HFONT                   mhDefFont;          // DefaultFont
    HPALETTE                mhDefPal;           // DefaultPalette
    COLORREF                mnTextColor;        // TextColor
    RGNDATA*                mpClipRgnData;      // ClipRegion-Data
    RGNDATA*                mpStdClipRgnData;   // Cache Standard-ClipRegion-Data
    int                     mnPenWidth;         // line width

public:
    // Return HFONT, and whether the font is for vertical writing ( prefixed with '@' )
    // and tmDescent value for adjusting offset in vertical writing mode.
    std::tuple<HFONT,bool,sal_Int32> ImplDoSetFont(HDC hDC, FontSelectPattern const & i_rFont, const PhysicalFontFace * i_pFontFace, HFONT& o_rOldFont);

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


public:
    explicit WinSalGraphics(WinSalGraphics::Type eType, bool bScreen, HWND hWnd,
                            SalGeometryProvider *pProvider);
    virtual ~WinSalGraphics() override;

    SalGraphicsImpl* GetImpl() const override;
    bool isPrinter() const;
    bool isVirtualDevice() const;
    bool isWindow() const;
    bool isScreen() const;

    void setHWND(HWND hWnd);
    void Flush();

protected:
    virtual bool        setClipRegion( const vcl::Region& ) override;
    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void        drawPixel( tools::Long nX, tools::Long nY ) override;
    virtual void        drawPixel( tools::Long nX, tools::Long nY, Color nColor ) override;
    virtual void        drawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2 ) override;
    virtual void        drawRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) override;
    virtual void        drawPolyLine( sal_uInt32 nPoints, const Point* pPtAry ) override;
    virtual void        drawPolygon( sal_uInt32 nPoints, const Point* pPtAry ) override;
    virtual void        drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, const Point** pPtAry ) override;
    virtual bool        drawPolyPolygon(
        const basegfx::B2DHomMatrix& rObjectToDevice,
        const basegfx::B2DPolyPolygon&,
        double fTransparency) override;
    virtual bool        drawPolyLine(
        const basegfx::B2DHomMatrix& rObjectToDevice,
        const basegfx::B2DPolygon&,
        double fTransparency,
        double fLineWidth,
        const std::vector< double >* pStroke, // MM01
        basegfx::B2DLineJoin,
        css::drawing::LineCap,
        double fMiterMinimumAngle,
        bool bPixelSnapHairline) override;
    virtual bool        drawPolyLineBezier( sal_uInt32 nPoints, const Point* pPtAry, const PolyFlags* pFlgAry ) override;
    virtual bool        drawPolygonBezier( sal_uInt32 nPoints, const Point* pPtAry, const PolyFlags* pFlgAry ) override;
    virtual bool        drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints, const Point* const* pPtAry, const PolyFlags* const* pFlgAry ) override;
    virtual bool        drawGradient( const tools::PolyPolygon&, const Gradient& ) override;
    virtual bool        implDrawGradient(basegfx::B2DPolyPolygon const & rPolyPolygon, SalGradient const & rGradient) override;

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void        copyArea( tools::Long nDestX, tools::Long nDestY, tools::Long nSrcX, tools::Long nSrcY, tools::Long nSrcWidth,
                                  tools::Long nSrcHeight, bool bWindowInvalidate ) override;

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void        copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) override;
    virtual void        drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap ) override;
    virtual void        drawBitmap( const SalTwoRect& rPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap ) override;
    virtual void        drawMask( const SalTwoRect& rPosAry,
                                  const SalBitmap& rSalBitmap,
                                  Color nMaskColor ) override;

    virtual std::shared_ptr<SalBitmap> getBitmap( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) override;
    virtual Color       getPixel( tools::Long nX, tools::Long nY ) override;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void        invert( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, SalInvert nFlags) override;
    virtual void        invert( sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags ) override;

    virtual bool        drawEPS( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, void* pPtr, sal_uInt32 nSize ) override;

    // native widget rendering methods that require mirroring
protected:
    virtual bool        isNativeControlSupported( ControlType nType, ControlPart nPart ) override;
    virtual bool        hitTestNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
                                              const Point& aPos, bool& rIsInside ) override;
    virtual bool        drawNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue,
                                           const OUString& aCaption, const Color& rBackgroundColor ) override;
    virtual bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const OUString& aCaption,
                                                tools::Rectangle &rNativeBoundingRegion, tools::Rectangle &rNativeContentRegion ) override;

public:
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
                           const SalBitmap* pAlphaBitmap,
                           double fAlpha) override;

    virtual bool       hasFastDrawTransformedBitmap() const override;

    virtual bool       drawAlphaRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, sal_uInt8 nTransparency ) override;

private:
    // local helpers

    void                    DrawTextLayout(const GenericSalLayout&, HDC, bool bUseDWrite);

public:
    // public SalGraphics methods, the interface to the independent vcl part

    // get device resolution
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;
    // get the depth of the device
    virtual sal_uInt16          GetBitCount() const override;
    // get the width of the device
    virtual tools::Long            GetGraphicsWidth() const override;

    // set the clip region to empty
    virtual void            ResetClipRegion() override;

    // set the line color to transparent (= don't draw lines)
    virtual void            SetLineColor() override;
    // set the line color to a specific color
    virtual void            SetLineColor( Color nColor ) override;
    // set the fill color to transparent (= don't fill)
    virtual void            SetFillColor() override;
    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void            SetFillColor( Color nColor ) override;
    // enable/disable XOR drawing
    virtual void            SetXORMode( bool bSet, bool ) override;
    // set line color for raster operations
    virtual void            SetROPLineColor( SalROPColor nROPColor ) override;
    // set fill color for raster operations
    virtual void            SetROPFillColor( SalROPColor nROPColor ) override;
    // set the text color to a specific color
    virtual void            SetTextColor( Color nColor ) override;
    // set the font
    virtual void            SetFont( LogicalFontInstance*, int nFallbackLevel ) override;
    // get the current font's metrics
    virtual void            GetFontMetric( ImplFontMetricDataRef&, int nFallbackLevel ) override;
    // get the repertoire of the current font
    virtual FontCharMapRef  GetFontCharMap() const override;
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
    // parameters: rToFile: contains an osl file URL to write the subset to
    //             pFont: describes from which font to create a subset
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             pWidths: the advance widths of the corresponding glyphs (in PS font units)
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

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void* GetEmbedFontData(const PhysicalFontFace*, tools::Long* pDataLen) override;
    // frees the font data again
    virtual void            FreeEmbedFontData( const void* pData, tools::Long nDataLen ) override;
    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            std::vector< sal_Int32 >& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) override;

    virtual std::unique_ptr<GenericSalLayout>
                            GetTextLayout(int nFallbackLevel) override;
    virtual void            DrawTextLayout( const GenericSalLayout& ) override;

    virtual bool            supportsOperation( OutDevSupportType ) const override;

    virtual SystemGraphicsData GetGraphicsData() const override;

    /// Update settings based on the platform values
    static void updateSettingsNative( AllSettings& rSettings );
};

// Init/Deinit Graphics
void    ImplUpdateSysColorEntries();
int     ImplIsSysColorEntry( Color nColor );
void    ImplGetLogFontFromFontSelect( const FontSelectPattern&,
            const PhysicalFontFace*, LOGFONTW& );

#define MAX_64KSALPOINTS    ((((sal_uInt16)0xFFFF)-8)/sizeof(POINTS))

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
