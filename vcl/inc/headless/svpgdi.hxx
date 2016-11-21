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

#include <osl/endian.h>
#include <basebmp/bitmapdevice.hxx>
#include <basebmp/color.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/metric.hxx>
#include <config_cairo_canvas.h>

#include "salgdi.hxx"
#include "sallayout.hxx"
#include "svpcairotextrender.hxx"

#ifdef IOS
#define SvpSalGraphics AquaSalGraphics
#else

//Using formats that match cairo's formats. For android we patch cairo,
//which is internal in that case, to swap the rgb components so that
//cairo then matches the OpenGL GL_RGBA format so we can use it there
//where we don't have GL_BGRA support.
#ifdef ANDROID
#   define SVP_CAIRO_FORMAT basebmp::Format::ThirtyTwoBitTcMaskRGBA
#else
#   define SVP_CAIRO_FORMAT basebmp::Format::ThirtyTwoBitTcMaskBGRA
#endif

class GlyphCache;
class ServerFont;
typedef struct _cairo cairo_t;
typedef struct _cairo_surface cairo_surface_t;

class VCL_DLLPUBLIC SvpSalGraphics : public SalGraphics
{
    basebmp::BitmapDeviceSharedPtr       m_aDevice;
    basebmp::BitmapDeviceSharedPtr       m_aOrigDevice;

    basebmp::BitmapDeviceSharedPtr       m_aClipMap;

    bool                                 m_bUseLineColor;
    basebmp::Color                       m_aLineColor;
    bool                                 m_bUseFillColor;
    basebmp::Color                       m_aFillColor;

    basebmp::DrawMode                    m_aDrawMode;

public:
    static GlyphCache& getPlatformGlyphCache();
    void setDevice(basebmp::BitmapDeviceSharedPtr& rDevice);

private:
    bool                                 m_bClipSetup;
    struct ClipUndoHandle {
        SvpSalGraphics                &m_rGfx;
        basebmp::BitmapDeviceSharedPtr m_aDevice;
        ClipUndoHandle( SvpSalGraphics *pGfx ) : m_rGfx( *pGfx ) {}
        ~ClipUndoHandle();
    };
    bool isClippedSetup( const basegfx::B2IBox &aRange, ClipUndoHandle &rUndo );
    void ensureClip();

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

public:
    SvpSalGraphics();
    virtual ~SvpSalGraphics();

    virtual SalGraphicsImpl* GetImpl() const override { return nullptr; };
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;
    virtual sal_uInt16      GetBitCount() const override;
    virtual long            GetGraphicsWidth() const override;

    virtual void            ResetClipRegion() override;
    virtual bool            setClipRegion( const vcl::Region& ) override;

    virtual void            SetLineColor() override;
    virtual void            SetLineColor( SalColor nSalColor ) override;
    virtual void            SetFillColor() override;
    virtual void            SetFillColor( SalColor nSalColor ) override;

    virtual void            SetXORMode( bool bSet, bool ) override;

    virtual void            SetROPLineColor( SalROPColor nROPColor ) override;
    virtual void            SetROPFillColor( SalROPColor nROPColor ) override;

    virtual void            SetTextColor( SalColor nSalColor ) override;
    virtual sal_uInt16      SetFont( FontSelectPattern*, int nFallbackLevel ) override;
    virtual void            GetFontMetric( ImplFontMetricData*, int nFallbackLevel ) override;
    virtual const FontCharMapPtr GetFontCharMap() const override;
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
    virtual const Ucs2SIntMap* GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded, std::set<sal_Unicode> const** ) override;
    virtual const void*     GetEmbedFontData( const PhysicalFontFace*,
                                              const sal_Ucs* pUnicodes,
                                              sal_Int32* pWidths,
                                              size_t nLen,
                                              FontSubsetInfo& rInfo,
                                              long* pDataLen ) override;
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen ) override;
    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) override;
    virtual bool            GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& ) override;
    virtual bool            GetGlyphOutline( sal_GlyphId nIndex, ::basegfx::B2DPolyPolygon& ) override;
    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) override;
    virtual void            DrawServerFontLayout( const ServerFontLayout& ) override;
    virtual bool            supportsOperation( OutDevSupportType ) const override;
    virtual void            drawPixel( long nX, long nY ) override;
    virtual void            drawPixel( long nX, long nY, SalColor nSalColor ) override;
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 ) override;
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight ) override;
    virtual bool            drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency ) override;
    virtual bool            drawPolyLine( const ::basegfx::B2DPolygon&,
                                          double fTransparency,
                                          const ::basegfx::B2DVector& rLineWidths,
                                          basegfx::B2DLineJoin,
                                          css::drawing::LineCap) override;
    virtual void            drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
    virtual void            drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
    virtual void            drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry ) override;
    virtual bool        drawPolyLineBezier( sal_uInt32 nPoints,
                                                const SalPoint* pPtAry,
                                                const sal_uInt8* pFlgAry ) override;
    virtual bool        drawPolygonBezier( sal_uInt32 nPoints,
                                               const SalPoint* pPtAry,
                                               const sal_uInt8* pFlgAry ) override;
    virtual bool        drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const sal_uInt8* const* pFlgAry ) override;
    virtual bool            drawGradient( const tools::PolyPolygon&, const Gradient& ) override { return false; };

    virtual void            copyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      sal_uInt16 nFlags ) override;
    virtual void            copyBits( const SalTwoRect& rPosAry,
                                      SalGraphics* pSrcGraphics ) override;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap ) override;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rTransparentBitmap ) override;
    virtual void            drawMask( const SalTwoRect& rPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor ) override;
    virtual SalBitmap*      getBitmap( long nX, long nY, long nWidth, long nHeight ) override;
    virtual SalColor        getPixel( long nX, long nY ) override;
    virtual void            invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags ) override;
    virtual void            invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) override;

    virtual bool        drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize ) override;

    virtual SystemGraphicsData GetGraphicsData() const override;

#if ENABLE_CAIRO_CANVAS
    virtual bool            SupportsCairo() const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const override;
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const override;
    virtual css::uno::Any   GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const override;
    virtual SystemFontData  GetSysFontData( int nFallbacklevel ) const override;
#endif // ENABLE_CAIRO_CANVAS

    cairo_t*                getCairoContext() const;
    static cairo_surface_t* createCairoSurface(const basebmp::BitmapDeviceSharedPtr& rBuffer);
    static cairo_t*         createCairoContext(const basebmp::BitmapDeviceSharedPtr& rBuffer);
    void                    clipRegion(cairo_t* cr);
};

#endif

#endif // INCLUDED_VCL_INC_HEADLESS_SVPGDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
