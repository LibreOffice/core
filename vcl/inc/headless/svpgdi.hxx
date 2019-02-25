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

#ifndef INCLUDED_VCL_INC_HEADLESS_SVPGDI_HXX
#define INCLUDED_VCL_INC_HEADLESS_SVPGDI_HXX

#ifdef IOS
#error This file is not for iOS
#endif

#include <osl/endian.h>
#include <vcl/sysdata.hxx>
#include <vcl/metric.hxx>
#include <config_cairo_canvas.h>

#include <salgdi.hxx>
#include <sallayout.hxx>
#include "svpcairotextrender.hxx"
#include <impfontmetricdata.hxx>

#include <cairo.h>

//Using formats that match cairo's formats. For android we patch cairo,
//which is internal in that case, to swap the rgb components so that
//cairo then matches the OpenGL GL_RGBA format so we can use it there
//where we don't have GL_BGRA support.
// SVP_24BIT_FORMAT is used to store 24-bit images in 3-byte pixels to conserve memory.
#if defined ANDROID
#   define SVP_24BIT_FORMAT (ScanlineFormat::N24BitTcRgb | ScanlineFormat::TopDown)
#   define SVP_CAIRO_FORMAT (ScanlineFormat::N32BitTcRgba | ScanlineFormat::TopDown)
#   define SVP_CAIRO_BLUE 1
#   define SVP_CAIRO_GREEN 2
#   define SVP_CAIRO_RED 0
#   define SVP_CAIRO_ALPHA 3
#elif defined OSL_BIGENDIAN
#   define SVP_24BIT_FORMAT (ScanlineFormat::N24BitTcRgb | ScanlineFormat::TopDown)
#   define SVP_CAIRO_FORMAT (ScanlineFormat::N32BitTcArgb | ScanlineFormat::TopDown)
#   define SVP_CAIRO_BLUE 3
#   define SVP_CAIRO_GREEN 2
#   define SVP_CAIRO_RED 1
#   define SVP_CAIRO_ALPHA 0
#else
#   define SVP_24BIT_FORMAT (ScanlineFormat::N24BitTcBgr | ScanlineFormat::TopDown)
#   define SVP_CAIRO_FORMAT (ScanlineFormat::N32BitTcBgra | ScanlineFormat::TopDown)
#   define SVP_CAIRO_BLUE 0
#   define SVP_CAIRO_GREEN 1
#   define SVP_CAIRO_RED 2
#   define SVP_CAIRO_ALPHA 3
#endif

struct BitmapBuffer;
class GlyphCache;
class FreetypeFont;
typedef struct _cairo cairo_t;
typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo_user_data_key cairo_user_data_key_t;

enum class PaintMode { Over, Xor };

typedef void (*damageHandler)(void* handle,
                              sal_Int32 nExtentsX, sal_Int32 nExtentsY,
                              sal_Int32 nExtentsWidth, sal_Int32 nExtentsHeight);

struct VCL_DLLPUBLIC DamageHandler
{
    void *handle;
    damageHandler damaged;
};

class VCL_DLLPUBLIC SvpSalGraphics : public SalGraphics
{
    cairo_surface_t*               m_pSurface;
    basegfx::B2IVector             m_aFrameSize;
    double                         m_fScale;
    Color                          m_aLineColor;
    Color                          m_aFillColor;
    PaintMode                      m_ePaintMode;

public:
    static GlyphCache& getPlatformGlyphCache();
    void setSurface(cairo_surface_t* pSurface, const basegfx::B2IVector& rSize);
    cairo_surface_t* getSurface() const { return m_pSurface; }
    static cairo_user_data_key_t* getDamageKey();

    static void clipRegion(cairo_t* cr, const vcl::Region& rClipRegion);

    // need this static version of ::drawPolyLine for usage from
    // vcl/unx/generic/gdi/salgdi.cxx. It gets wrapped by
    // ::drawPolyLine with some added parameters (see there)
    static bool drawPolyLine(
        cairo_t* cr,
        basegfx::B2DRange* pExtents,
        const Color& rLineColor,
        bool bAntiAliasB2DDraw,
        const basegfx::B2DHomMatrix& rObjectToDevice,
        const basegfx::B2DPolygon& rPolyLine,
        double fTransparency,
        const basegfx::B2DVector& rLineWidths,
        basegfx::B2DLineJoin eLineJoin,
        css::drawing::LineCap eLineCap,
        double fMiterMinimumAngle,
        bool bPixelSnapHairline);

    void copySource(const SalTwoRect& rTR, cairo_surface_t* source);
    void copyWithOperator(const SalTwoRect& rTR, cairo_surface_t* source,
                          cairo_operator_t eOp = CAIRO_OPERATOR_SOURCE);

private:
    void invert(const basegfx::B2DPolygon &rPoly, SalInvert nFlags);
    void applyColor(cairo_t *cr, Color rColor);

protected:
    vcl::Region                         m_aClipRegion;
    SvpCairoTextRender                  m_aTextRenderImpl;

protected:
    virtual bool blendBitmap( const SalTwoRect&, const SalBitmap& rBitmap ) override;
    virtual bool blendAlphaBitmap( const SalTwoRect&,
                                   const SalBitmap& rSrcBitmap,
                                   const SalBitmap& rMaskBitmap,
                                   const SalBitmap& rAlphaBitmap ) override;
    virtual bool drawAlphaBitmap( const SalTwoRect&, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap ) override;
    virtual bool drawTransformedBitmap(
        const basegfx::B2DPoint& rNull,
        const basegfx::B2DPoint& rX,
        const basegfx::B2DPoint& rY,
        const SalBitmap& rSourceBitmap,
        const SalBitmap* pAlphaBitmap) override;
    virtual bool drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency ) override;

    cairo_t* createTmpCompatibleCairoContext() const;

public:
    SvpSalGraphics();
    virtual ~SvpSalGraphics() override;

    virtual SalGraphicsImpl* GetImpl() const override { return nullptr; };
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;
    virtual sal_uInt16      GetBitCount() const override;
    virtual long            GetGraphicsWidth() const override;

    virtual void            ResetClipRegion() override;
    virtual bool            setClipRegion( const vcl::Region& ) override;

    virtual void            SetLineColor() override;
    virtual void            SetLineColor( Color nColor ) override;
    virtual void            SetFillColor() override;
    virtual void            SetFillColor( Color nColor ) override;

    virtual void            SetXORMode( bool bSet, bool ) override;

    virtual void            SetROPLineColor( SalROPColor nROPColor ) override;
    virtual void            SetROPFillColor( SalROPColor nROPColor ) override;

    virtual void            SetTextColor( Color nColor ) override;
    virtual void            SetFont(LogicalFontInstance*, int nFallbackLevel) override;
    virtual void            GetFontMetric( ImplFontMetricDataRef&, int nFallbackLevel ) override;
    virtual const FontCharMapRef GetFontCharMap() const override;
    virtual bool GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const override;
    virtual void            GetDevFontList( PhysicalFontCollection* ) override;
    virtual void ClearDevFontCache() override;
    virtual bool            AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) override;
    virtual bool        CreateFontSubset( const OUString& rToFile,
                                              const PhysicalFontFace*,
                                              const sal_GlyphId* pGlyphIds,
                                              const sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo
                                              ) override;
    virtual const void*     GetEmbedFontData(const PhysicalFontFace*, long* pDataLen) override;
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen ) override;
    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            std::vector< sal_Int32 >& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) override;
    virtual std::unique_ptr<GenericSalLayout>
                            GetTextLayout(int nFallbackLevel) override;
    virtual void            DrawTextLayout( const GenericSalLayout& ) override;
    virtual bool            supportsOperation( OutDevSupportType ) const override;
    virtual void            drawPixel( long nX, long nY ) override;
    virtual void            drawPixel( long nX, long nY, Color nColor ) override;
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 ) override;
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight ) override;

    virtual bool            drawPolyPolygon(
                                const basegfx::B2DHomMatrix& rObjectToDevice,
                                const basegfx::B2DPolyPolygon&,
                                double fTransparency ) override;

    virtual bool            drawPolyLine(
                                const basegfx::B2DHomMatrix& rObjectToDevice,
                                const basegfx::B2DPolygon&,
                                double fTransparency,
                                const basegfx::B2DVector& rLineWidths,
                                basegfx::B2DLineJoin,
                                css::drawing::LineCap,
                                double fMiterMinimumAngle,
                                bool bPixelSnapHairline) override;
    virtual void            drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
    virtual void            drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
    virtual void            drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry ) override;
    virtual bool        drawPolyLineBezier( sal_uInt32 nPoints,
                                                const SalPoint* pPtAry,
                                                const PolyFlags* pFlgAry ) override;
    virtual bool        drawPolygonBezier( sal_uInt32 nPoints,
                                               const SalPoint* pPtAry,
                                               const PolyFlags* pFlgAry ) override;
    virtual bool        drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const PolyFlags* const* pFlgAry ) override;
    virtual bool            drawGradient( const tools::PolyPolygon&, const Gradient& ) override { return false; };

    virtual void            copyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      bool bWindowInvalidate) override;
    virtual void            copyBits( const SalTwoRect& rPosAry,
                                      SalGraphics* pSrcGraphics ) override;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap ) override;
    void                    drawBitmap( const SalTwoRect& rPosAry,
                                        const BitmapBuffer* pBuffer,
                                        cairo_operator_t eOp );
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rTransparentBitmap ) override;
    virtual void            drawMask( const SalTwoRect& rPosAry,
                                      const SalBitmap& rSalBitmap,
                                      Color nMaskColor ) override;
    virtual std::shared_ptr<SalBitmap> getBitmap( long nX, long nY, long nWidth, long nHeight ) override;
    virtual Color           getPixel( long nX, long nY ) override;
    virtual void            invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags ) override;
    virtual void            invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) override;

    virtual bool        drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uInt32 nSize ) override;

    virtual SystemGraphicsData GetGraphicsData() const override;

    // Native Widget Drawing interface
    bool IsNativeControlSupported(ControlType eType, ControlPart ePart) override;

    bool hitTestNativeControl(ControlType eType, ControlPart ePart,
                               const tools::Rectangle& rBoundingControlRegion,
                               const Point& rPosition, bool& rIsInside) override;

    bool drawNativeControl(ControlType eType, ControlPart ePart,
                           const tools::Rectangle& rBoundingControlRegion,
                           ControlState eState, const ImplControlValue& aValue,
                           const OUString& aCaptions) override;

    bool getNativeControlRegion(ControlType eType, ControlPart ePart,
                                 const tools::Rectangle& rBoundingControlRegion,
                                 ControlState eState,
                                 const ImplControlValue& aValue,
                                 const OUString& aCaption,
                                 tools::Rectangle& rNativeBoundingRegion,
                                 tools::Rectangle& rNativeContentRegion) override;

    virtual void updateSettings(AllSettings& rSettings);

#if ENABLE_CAIRO_CANVAS
    virtual bool            SupportsCairo() const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const override;
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const override;
    virtual css::uno::Any   GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const override;
    virtual SystemFontData  GetSysFontData( int nFallbacklevel ) const override;
#endif // ENABLE_CAIRO_CANVAS

    cairo_t*                getCairoContext(bool bXorModeAllowed) const;
    void                    releaseCairoContext(cairo_t* cr, bool bXorModeAllowed, const basegfx::B2DRange& rExtents) const;
    static cairo_surface_t* createCairoSurface(const BitmapBuffer *pBuffer);
    void                    clipRegion(cairo_t* cr);
};

#endif // INCLUDED_VCL_INC_HEADLESS_SVPGDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
