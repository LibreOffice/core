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

#pragma once

#include <sal/config.h>

#include <sallayout.hxx>
#include <salgeom.hxx>
#include <salgdi.hxx>
#include <font/LogicalFontInstance.hxx>
#include <fontattributes.hxx>
#include <font/PhysicalFontFace.hxx>
#include <impfont.hxx>
#include <vcl/fontcapabilities.hxx>
#include <vcl/fontcharmap.hxx>
#include <systools/win32/comtools.hxx>

#include <memory>
#include <unordered_set>

#ifndef INCLUDED_PRE_POST_WIN_H
#define INCLUDED_PRE_POST_WIN_H
#  include <prewin.h>
#  include <postwin.h>
#endif

#include <hb-ot.h>
#include <dwrite.h>

namespace vcl::font
{
class PhysicalFontCollection;
class FontSelectPattern;
}
class WinFontInstance;
class ImplFontAttrCache;
class SalGraphicsImpl;
class WinSalGraphicsImplBase;
class FontMetricData;

#define RGB_TO_PALRGB(nRGB)         ((nRGB)|0x02000000)
#define PALRGB_TO_RGB(nPalRGB)      ((nPalRGB)&0x00ffffff)

// win32 specific physically available font face
class WinFontFace final : public vcl::font::PhysicalFontFace
{
public:
    explicit                WinFontFace(const ENUMLOGFONTEXW&, const NEWTEXTMETRICW&);
                            ~WinFontFace() override;

    rtl::Reference<LogicalFontInstance> CreateFontInstance( const vcl::font::FontSelectPattern& ) const override;
    sal_IntPtr              GetFontId() const override;
    void                    SetFontId( sal_IntPtr nId ) { mnId = nId; }

    BYTE                    GetCharSet() const          { return meWinCharSet; }
    BYTE                    GetPitchAndFamily() const   { return mnPitchAndFamily; }

    hb_blob_t*              GetHbTable(hb_tag_t nTag) const override;

    const std::vector<hb_variation_t>& GetVariations(const LogicalFontInstance&) const override;

private:
    sal_IntPtr              mnId;

    BYTE                    meWinCharSet;
    BYTE                    mnPitchAndFamily;
    LOGFONTW                maLogFont;
    mutable sal::systools::COMReference<IDWriteFontFace> mxDWFontFace;
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

/**
 * WinSalGraphics never owns the HDC it uses to draw, because the HDC can have
 * various origins with different ways to correctly free it. And WinSalGraphics
 * stores all default values (mhDef*) of the HDC, which must be restored when
 * the HDC changes (setHDC) or the SalGraphics is destructed. So think of the
 * HDC in terms of Rust's Borrowing semantics.
 */
class WinSalGraphics : public SalGraphics
{
    friend class WinSalGraphicsImpl;
    friend class ScopedFont;

protected:
    std::unique_ptr<SalGraphicsImpl> mpImpl;
    WinSalGraphicsImplBase * mWinSalGraphicsImplBase;

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

    // just call both from setHDC!
    void InitGraphics();
    void DeInitGraphics();

public:
    // Return HFONT, and whether the font is for vertical writing ( prefixed with '@' )
    // and tmDescent value for adjusting offset in vertical writing mode.
    std::tuple<HFONT,bool,sal_Int32> ImplDoSetFont(HDC hDC, vcl::font::FontSelectPattern const & i_rFont, const vcl::font::PhysicalFontFace * i_pFontFace, HFONT& o_rOldFont);

    HDC getHDC() const { return mhLocalDC; }
    // NOTE: this doesn't transfer ownership! See class comment.
    void setHDC(HDC aNew);

    HPALETTE getDefPal() const;
    // returns the result from RealizePalette, otherwise 0 on success or GDI_ERROR
    UINT setPalette(HPALETTE, BOOL bForceBkgd = TRUE);

    HRGN getRegion() const;


    enum Type
    {
        PRINTER,
        VIRTUAL_DEVICE,
        WINDOW,
        SCREEN
    };

    static IDWriteFactory* getDWriteFactory();
    static IDWriteGdiInterop* getDWriteGdiInterop();

public:

    HWND gethWnd();


public:
    explicit WinSalGraphics(WinSalGraphics::Type eType, bool bScreen, HWND hWnd,
                            SalGeometryProvider *pProvider);
    virtual ~WinSalGraphics() override;

    SalGraphicsImpl* GetImpl() const override;
    WinSalGraphicsImplBase * getWinSalGraphicsImplBase() const { return mWinSalGraphicsImplBase; }
    bool isPrinter() const;
    bool isVirtualDevice() const;
    bool isWindow() const;
    bool isScreen() const;

    void setHWND(HWND hWnd);
    void Flush();

protected:
    virtual void        setClipRegion( const vcl::Region& ) override;
    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void        drawPixel( tools::Long nX, tools::Long nY ) override;
    virtual void        drawPixel( tools::Long nX, tools::Long nY, Color nColor ) override;
    virtual void        drawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2 ) override;
    virtual void        drawRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) override;
    virtual void        drawPolyLine( sal_uInt32 nPoints, const Point* pPtAry ) override;
    virtual void        drawPolygon( sal_uInt32 nPoints, const Point* pPtAry ) override;
    virtual void        drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, const Point** pPtAry ) override;
    virtual void        drawPolyPolygon(
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

    void DrawTextLayout(const GenericSalLayout&, HDC, bool bUseDWrite, bool bRenderingModeNatural);

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
    virtual void            GetFontMetric( FontMetricDataRef&, int nFallbackLevel ) override;
    // get the repertoire of the current font
    virtual FontCharMapRef  GetFontCharMap() const override;
    // get the layout capabilities of the current font
    virtual bool GetFontCapabilities(vcl::FontCapabilities &rGetFontCapabilities) const override;
    // graphics must fill supplied font list
    virtual void            GetDevFontList( vcl::font::PhysicalFontCollection* ) override;
    // graphics must drop any cached font info
    virtual void            ClearDevFontCache() override;
    virtual bool            AddTempDevFont( vcl::font::PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) override;

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
void    ImplGetLogFontFromFontSelect( const vcl::font::FontSelectPattern&,
            const vcl::font::PhysicalFontFace*, LOGFONTW& );

#define MAX_64KSALPOINTS    ((((sal_uInt16)0xFFFF)-8)/sizeof(POINTS))

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
