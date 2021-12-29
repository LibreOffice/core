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

#ifdef IOS
#error This file is not for iOS
#endif

#include <sal/config.h>
#include <config_features.h>

#include <osl/endian.h>
#include <vcl/sysdata.hxx>
#include <config_cairo_canvas.h>

#include <font/PhysicalFontFace.hxx>
#include <salgdi.hxx>
#include <sallayout.hxx>
#include "svpcairotextrender.hxx"
#include <impfontmetricdata.hxx>

#include <headless/SvpGraphicsBackend.hxx>
#include <headless/CairoCommon.hxx>

struct BitmapBuffer;
class FreetypeFont;

class VCL_DLLPUBLIC SvpSalGraphics : public SalGraphicsAutoDelegateToImpl
{
    CairoCommon m_aCairoCommon;

public:
    void setSurface(cairo_surface_t* pSurface, const basegfx::B2IVector& rSize);
    cairo_surface_t* getSurface() const { return m_aCairoCommon.m_pSurface; }
    static cairo_user_data_key_t* getDamageKey()
    {
        return CairoCommon::getDamageKey();
    }

    static void clipRegion(cairo_t* cr, const vcl::Region& rClipRegion);

    // need this static version of ::drawPolyLine for usage from
    // vcl/unx/generic/gdi/salgdi.cxx. It gets wrapped by
    // ::drawPolyLine with some added parameters (see there)
    static bool drawPolyLine(
        cairo_t* cr,
        basegfx::B2DRange* pExtents,
        const Color& rLineColor,
        bool bAntiAlias,
        const basegfx::B2DHomMatrix& rObjectToDevice,
        const basegfx::B2DPolygon& rPolyLine,
        double fTransparency,
        double fLineWidth,
        const std::vector< double >* pStroke, // MM01
        basegfx::B2DLineJoin eLineJoin,
        css::drawing::LineCap eLineCap,
        double fMiterMinimumAngle,
        bool bPixelSnapHairline);

    void copySource(const SalTwoRect& rTR, cairo_surface_t* source);
    void copyWithOperator(const SalTwoRect& rTR, cairo_surface_t* source,
                          cairo_operator_t eOp = CAIRO_OPERATOR_SOURCE);

private:
    void invert(const basegfx::B2DPolygon &rPoly, SalInvert nFlags);
    void applyColor(cairo_t *cr, Color rColor, double fTransparency = 0.0);

protected:
    SvpCairoTextRender                  m_aTextRenderImpl;
    std::unique_ptr<SvpGraphicsBackend> m_pBackend;

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
        const SalBitmap* pAlphaBitmap,
        double fAlpha) override;
    virtual bool hasFastDrawTransformedBitmap() const override;
    virtual bool drawAlphaRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, sal_uInt8 nTransparency ) override;

    cairo_t* createTmpCompatibleCairoContext() const;

public:
    SvpSalGraphics();
    virtual ~SvpSalGraphics() override;

    virtual SalGraphicsImpl* GetImpl() const override { return m_pBackend.get(); }

    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;

    virtual void            SetTextColor( Color nColor ) override;
    virtual void            SetFont(LogicalFontInstance*, int nFallbackLevel) override;
    virtual void            GetFontMetric( ImplFontMetricDataRef&, int nFallbackLevel ) override;
    virtual FontCharMapRef  GetFontCharMap() const override;
    virtual bool GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const override;
    virtual void            GetDevFontList( vcl::font::PhysicalFontCollection* ) override;
    virtual void ClearDevFontCache() override;
    virtual bool            AddTempDevFont( vcl::font::PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) override;
    virtual bool        CreateFontSubset( const OUString& rToFile,
                                              const vcl::font::PhysicalFontFace*,
                                              const sal_GlyphId* pGlyphIds,
                                              const sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo
                                              ) override;
    virtual const void*     GetEmbedFontData(const vcl::font::PhysicalFontFace*, tools::Long* pDataLen) override;
    virtual void            FreeEmbedFontData( const void* pData, tools::Long nDataLen ) override;
    virtual void            GetGlyphWidths( const vcl::font::PhysicalFontFace*,
                                            bool bVertical,
                                            std::vector< sal_Int32 >& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) override;
    virtual std::unique_ptr<GenericSalLayout>
                            GetTextLayout(int nFallbackLevel) override;
    virtual void            DrawTextLayout( const GenericSalLayout& ) override;
    virtual bool            supportsOperation( OutDevSupportType ) const override;
    virtual void            drawPixel( tools::Long nX, tools::Long nY ) override;
    virtual void            drawPixel( tools::Long nX, tools::Long nY, Color nColor ) override;
    virtual void            drawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2 ) override;
    virtual void            drawRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) override;

    virtual bool            drawPolyPolygon(
                                const basegfx::B2DHomMatrix& rObjectToDevice,
                                const basegfx::B2DPolyPolygon&,
                                double fTransparency ) override;

    virtual bool            drawPolyLine(
                                const basegfx::B2DHomMatrix& rObjectToDevice,
                                const basegfx::B2DPolygon&,
                                double fTransparency,
                                double fLineWidth,
                                const std::vector< double >* pStroke, // MM01
                                basegfx::B2DLineJoin,
                                css::drawing::LineCap,
                                double fMiterMinimumAngle,
                                bool bPixelSnapHairline) override;
    virtual void            drawPolyLine( sal_uInt32 nPoints, const Point* pPtAry ) override;
    virtual void            drawPolygon( sal_uInt32 nPoints, const Point* pPtAry ) override;
    virtual void            drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             const Point** pPtAry ) override;
    virtual bool        drawPolyLineBezier( sal_uInt32 nPoints,
                                                const Point* pPtAry,
                                                const PolyFlags* pFlgAry ) override;
    virtual bool        drawPolygonBezier( sal_uInt32 nPoints,
                                               const Point* pPtAry,
                                               const PolyFlags* pFlgAry ) override;
    virtual bool        drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const Point* const* pPtAry,
                                                   const PolyFlags* const* pFlgAry ) override;

    virtual bool drawGradient(tools::PolyPolygon const & rPolyPolygon, Gradient const & rGradient) override;

    virtual bool implDrawGradient(basegfx::B2DPolyPolygon const & rPolyPolygon, SalGradient const & rGradient) override;

    virtual void            copyArea( tools::Long nDestX,
                                      tools::Long nDestY,
                                      tools::Long nSrcX,
                                      tools::Long nSrcY,
                                      tools::Long nSrcWidth,
                                      tools::Long nSrcHeight,
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
    virtual std::shared_ptr<SalBitmap> getBitmap( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) override;
    virtual Color           getPixel( tools::Long nX, tools::Long nY ) override;
    virtual void            invert( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, SalInvert nFlags ) override;
    virtual void            invert( sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags ) override;

    virtual bool        drawEPS( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, void* pPtr, sal_uInt32 nSize ) override;

    virtual SystemGraphicsData GetGraphicsData() const override;

#if ENABLE_CAIRO_CANVAS
    virtual bool            SupportsCairo() const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const override;
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const override;
    virtual css::uno::Any   GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const override;
#endif // ENABLE_CAIRO_CANVAS

    cairo_t* getCairoContext(bool bXorModeAllowed) const
    {
        return m_aCairoCommon.getCairoContext(bXorModeAllowed, getAntiAlias());
    }

    void releaseCairoContext(cairo_t* cr, bool bXorModeAllowed, const basegfx::B2DRange& rExtents) const
    {
        return m_aCairoCommon.releaseCairoContext(cr, bXorModeAllowed, rExtents);
    }

    static cairo_surface_t* createCairoSurface(const BitmapBuffer *pBuffer);
    void                    clipRegion(cairo_t* cr);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
