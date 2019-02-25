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

#ifndef INCLUDED_VCL_INC_GENERIC_GENPSPGRAPHICS_H
#define INCLUDED_VCL_INC_GENERIC_GENPSPGRAPHICS_H

#include <vcl/vclenum.hxx>
#include <config_cairo_canvas.h>

#include <unx/fontmanager.hxx>
#include <salgdi.hxx>
#include <sallayout.hxx>
#include <vclpluginapi.h>

class PhysicalFontFace;
class PhysicalFontCollection;

namespace psp { struct JobData; class PrinterGfx; }

class FreetypeFont;
class FontAttributes;
class SalInfoPrinter;
class GlyphCache;
class ImplFontMetricData;

class VCL_DLLPUBLIC GenPspGraphics : public SalGraphics
{
    psp::JobData*           m_pJobData;
    psp::PrinterGfx*        m_pPrinterGfx;

    FreetypeFont*           m_pFreetypeFont[ MAX_FALLBACK ];
public:
                            GenPspGraphics();
    virtual                ~GenPspGraphics() override;

    void                    Init( psp::JobData* pJob, psp::PrinterGfx* pGfx );

    // helper methods
    static const void *     DoGetEmbedFontData(psp::fontID aFont, long* pDataLen);
    static void             DoFreeEmbedFontData( const void* pData, long nLen );

    // helper methods for sharing with X11SalGraphics
    static void             DoGetGlyphWidths( psp::fontID aFont,
                                              bool bVertical,
                                              std::vector< sal_Int32 >& rWidths,
                                              Ucs2UIntMap& rUnicodeEnc );

    static FontAttributes Info2FontAttributes( const psp::FastPrintFontInfo& );
    static void             AnnounceFonts( PhysicalFontCollection*,
                                           const psp::FastPrintFontInfo& );

    // override all pure virtual methods
    virtual SalGraphicsImpl*GetImpl() const override { return nullptr; };
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
    virtual bool            GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const override;
    virtual void            GetDevFontList( PhysicalFontCollection* ) override;
    // graphics must drop any cached font info
    virtual void            ClearDevFontCache() override;
    virtual bool            AddTempDevFont( PhysicalFontCollection*,
                                            const OUString& rFileURL,
                                            const OUString& rFontName ) override;
    static bool             AddTempDevFontHelper( PhysicalFontCollection* pFontCollection,
                                                  const OUString& rFileURL,
                                                  const OUString& rFontName,
                                                  GlyphCache &rGC );

    virtual bool            CreateFontSubset( const OUString& rToFile,
                                              const PhysicalFontFace*,
                                              const sal_GlyphId* pGlyphIDs,
                                              const sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo ) override;
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
    virtual void            drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
    virtual void            drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
    virtual void            drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry ) override;

    virtual bool            drawPolyPolygon(
                                const basegfx::B2DHomMatrix& rObjectToDevice,
                                const basegfx::B2DPolyPolygon&,
                                double fTransparency) override;

    virtual bool            drawPolyLine(
                                const basegfx::B2DHomMatrix& rObjectToDevice,
                                const basegfx::B2DPolygon&,
                                double fTransparency,
                                const basegfx::B2DVector& rLineWidths,
                                basegfx::B2DLineJoin,
                                css::drawing::LineCap,
                                double fMiterMinimumAngle,
                                bool bPixelSnapHairline) override;
    virtual bool            drawPolyLineBezier( sal_uInt32 nPoints,
                                                const SalPoint* pPtAry,
                                                const PolyFlags* pFlgAry ) override;
    virtual bool            drawPolygonBezier( sal_uInt32 nPoints,
                                               const SalPoint* pPtAry,
                                               const PolyFlags* pFlgAry ) override;
    virtual bool            drawPolyPolygonBezier( sal_uInt32 nPoly,
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
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rTransparentBitmap ) override;
    virtual void            drawMask( const SalTwoRect& rPosAry,
                                      const SalBitmap& rSalBitmap,
                                      Color nMaskColor ) override;
    virtual std::shared_ptr<SalBitmap> getBitmap( long nX, long nY, long nWidth, long nHeight ) override;
    virtual Color           getPixel( long nX, long nY ) override;
    virtual void            invert( long nX, long nY, long nWidth, long nHeight,
                                    SalInvert nFlags ) override;
    virtual void            invert( sal_uInt32 nPoints, const SalPoint* pPtAry,
                                    SalInvert nFlags ) override;

    virtual bool            drawEPS( long nX, long nY, long nWidth, long nHeight,
                                     void* pPtr, sal_uInt32 nSize ) override;
    virtual bool            blendBitmap( const SalTwoRect&,
                                         const SalBitmap& rBitmap ) override;
    virtual bool            blendAlphaBitmap( const SalTwoRect&,
                                              const SalBitmap& rSrcBitmap,
                                              const SalBitmap& rMaskBitmap,
                                              const SalBitmap& rAlphaBitmap ) override;
    virtual bool            drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap ) override;
    virtual bool            drawTransformedBitmap( const basegfx::B2DPoint& rNull,
                                                   const basegfx::B2DPoint& rX,
                                                   const basegfx::B2DPoint& rY,
                                                   const SalBitmap& rSourceBitmap,
                                                   const SalBitmap* pAlphaBitmap) override;
    virtual bool            drawAlphaRect( long nX, long nY, long nWidth, long nHeight,
                                           sal_uInt8 nTransparency ) override;

    virtual SystemGraphicsData GetGraphicsData() const override;

#if ENABLE_CAIRO_CANVAS
    virtual bool            SupportsCairo() const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const override;
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const override;
    virtual css::uno::Any   GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const override;

    virtual SystemFontData  GetSysFontData( int nFallbacklevel ) const override;
#endif // ENABLE_CAIRO_CANVAS
};

#endif // INCLUDED_VCL_INC_GENERIC_GENPSPGRAPHICS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
