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

#include "fontmanager.hxx"
#include "salgdi.hxx"
#include "sallayout.hxx"
#include "vclpluginapi.h"

class PhysicalFontFace;
class PhysicalFontCollection;

namespace psp { struct JobData; class PrinterGfx; }

class ServerFont;
class ImplDevFontAttributes;
class SalInfoPrinter;

class VCL_DLLPUBLIC GenPspGraphics : public SalGraphics
{
 protected:
    psp::JobData*           m_pJobData;
    psp::PrinterGfx*        m_pPrinterGfx;

    ServerFont*             m_pServerFont[ MAX_FALLBACK ];
    bool                    m_bFontVertical;
    SalInfoPrinter*         m_pInfoPrinter;
public:
                            GenPspGraphics();
    virtual                ~GenPspGraphics();

    void                    Init( psp::JobData* pJob, psp::PrinterGfx* pGfx,
                                  SalInfoPrinter* pInfoPrinter );

    // helper methods
    static const void *     DoGetEmbedFontData ( psp::fontID aFont, const sal_Ucs* pUnicodes,
                                                 sal_Int32* pWidths, size_t nLen, FontSubsetInfo& rInfo,
                                                 long* pDataLen );
    static void             DoFreeEmbedFontData( const void* pData, long nLen );

    // helper methods for sharing with X11SalGraphics
    static const Ucs2SIntMap* DoGetFontEncodingVector( psp::fontID aFont,
                                                       const Ucs2OStrMap** pNonEncoded,
                                                       std::set<sal_Unicode> const** ppPriority);
    static void             DoGetGlyphWidths( psp::fontID aFont,
                                              bool bVertical,
                                              Int32Vector& rWidths,
                                              Ucs2UIntMap& rUnicodeEnc );

    static ImplDevFontAttributes Info2DevFontAttributes( const psp::FastPrintFontInfo& );
    static void             AnnounceFonts( PhysicalFontCollection*,
                                           const psp::FastPrintFontInfo& );

    // override all pure virtual methods
    virtual SalGraphicsImpl*GetImpl() const SAL_OVERRIDE { return NULL; };
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) SAL_OVERRIDE;
    virtual sal_uInt16      GetBitCount() const SAL_OVERRIDE;
    virtual long            GetGraphicsWidth() const SAL_OVERRIDE;

    virtual void            ResetClipRegion() SAL_OVERRIDE;
    virtual bool            setClipRegion( const vcl::Region& ) SAL_OVERRIDE;

    virtual void            SetLineColor() SAL_OVERRIDE;
    virtual void            SetLineColor( SalColor nSalColor ) SAL_OVERRIDE;
    virtual void            SetFillColor() SAL_OVERRIDE;
    virtual void            SetFillColor( SalColor nSalColor ) SAL_OVERRIDE;
    virtual void            SetXORMode( bool bSet, bool ) SAL_OVERRIDE;
    virtual void            SetROPLineColor( SalROPColor nROPColor ) SAL_OVERRIDE;
    virtual void            SetROPFillColor( SalROPColor nROPColor ) SAL_OVERRIDE;

    virtual void            SetTextColor( SalColor nSalColor ) SAL_OVERRIDE;
    virtual sal_uInt16      SetFont( FontSelectPattern*, int nFallbackLevel ) SAL_OVERRIDE;
    virtual void            GetFontMetric( ImplFontMetricData*, int nFallbackLevel ) SAL_OVERRIDE;
    virtual const FontCharMapPtr GetFontCharMap() const SAL_OVERRIDE;
    virtual bool            GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const SAL_OVERRIDE;
    virtual void            GetDevFontList( PhysicalFontCollection* ) SAL_OVERRIDE;
    // graphics must drop any cached font info
    virtual void            ClearDevFontCache() SAL_OVERRIDE;
    virtual bool            AddTempDevFont( PhysicalFontCollection*,
                                            const OUString& rFileURL,
                                            const OUString& rFontName ) SAL_OVERRIDE;

    virtual bool            CreateFontSubset( const OUString& rToFile,
                                              const PhysicalFontFace*,
                                              const sal_GlyphId* pGlyphIDs,
                                              const sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo ) SAL_OVERRIDE;
    virtual const Ucs2SIntMap* GetFontEncodingVector( const PhysicalFontFace*,
                                                      const Ucs2OStrMap** ppNonEncoded,
                                                      std::set<sal_Unicode> const** ppPriority) SAL_OVERRIDE;
    virtual const void*     GetEmbedFontData( const PhysicalFontFace*,
                                              const sal_Ucs* pUnicodes,
                                              sal_Int32* pWidths,
                                              size_t nLen,
                                              FontSubsetInfo& rInfo,
                                              long* pDataLen ) SAL_OVERRIDE;
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen ) SAL_OVERRIDE;
    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) SAL_OVERRIDE;
    virtual bool            GetGlyphBoundRect( sal_GlyphId, Rectangle& ) SAL_OVERRIDE;
    virtual bool            GetGlyphOutline( sal_GlyphId, basegfx::B2DPolyPolygon& ) SAL_OVERRIDE;
    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) SAL_OVERRIDE;
    virtual void            DrawServerFontLayout( const ServerFontLayout& ) SAL_OVERRIDE;
    virtual bool            supportsOperation( OutDevSupportType ) const SAL_OVERRIDE;
    virtual void            drawPixel( long nX, long nY ) SAL_OVERRIDE;
    virtual void            drawPixel( long nX, long nY, SalColor nSalColor ) SAL_OVERRIDE;
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 ) SAL_OVERRIDE;
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void            drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;
    virtual void            drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;
    virtual void            drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry ) SAL_OVERRIDE;
    virtual bool            drawPolyPolygon( const basegfx::B2DPolyPolygon&,
                                             double fTransparency ) SAL_OVERRIDE;
    virtual bool            drawPolyLine( const basegfx::B2DPolygon&,
                                          double fTransparency,
                                          const basegfx::B2DVector& rLineWidths,
                                          basegfx::B2DLineJoin,
                                          css::drawing::LineCap) SAL_OVERRIDE;
    virtual bool            drawPolyLineBezier( sal_uInt32 nPoints,
                                                const SalPoint* pPtAry,
                                                const sal_uInt8* pFlgAry ) SAL_OVERRIDE;
    virtual bool            drawPolygonBezier( sal_uInt32 nPoints,
                                               const SalPoint* pPtAry,
                                               const sal_uInt8* pFlgAry ) SAL_OVERRIDE;
    virtual bool            drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const sal_uInt8* const* pFlgAry ) SAL_OVERRIDE;
    virtual bool            drawGradient( const tools::PolyPolygon&, const Gradient& ) SAL_OVERRIDE { return false; };

    virtual void            copyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      sal_uInt16 nFlags ) SAL_OVERRIDE;
    virtual void            copyBits( const SalTwoRect& rPosAry,
                                      SalGraphics* pSrcGraphics ) SAL_OVERRIDE;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap ) SAL_OVERRIDE;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nTransparentColor ) SAL_OVERRIDE;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rTransparentBitmap ) SAL_OVERRIDE;
    virtual void            drawMask( const SalTwoRect& rPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor ) SAL_OVERRIDE;
    virtual SalBitmap*      getBitmap( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual SalColor        getPixel( long nX, long nY ) SAL_OVERRIDE;
    virtual void            invert( long nX, long nY, long nWidth, long nHeight,
                                    SalInvert nFlags ) SAL_OVERRIDE;
    virtual void            invert( sal_uInt32 nPoints, const SalPoint* pPtAry,
                                    SalInvert nFlags ) SAL_OVERRIDE;

    virtual bool            drawEPS( long nX, long nY, long nWidth, long nHeight,
                                     void* pPtr, sal_uIntPtr nSize ) SAL_OVERRIDE;
    virtual bool            blendBitmap( const SalTwoRect&,
                                         const SalBitmap& rBitmap ) SAL_OVERRIDE;
    virtual bool            blendAlphaBitmap( const SalTwoRect&,
                                              const SalBitmap& rSrcBitmap,
                                              const SalBitmap& rMaskBitmap,
                                              const SalBitmap& rAlphaBitmap ) SAL_OVERRIDE;
    virtual bool            drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap ) SAL_OVERRIDE;
    virtual bool            drawTransformedBitmap( const basegfx::B2DPoint& rNull,
                                                   const basegfx::B2DPoint& rX,
                                                   const basegfx::B2DPoint& rY,
                                                   const SalBitmap& rSourceBitmap,
                                                   const SalBitmap* pAlphaBitmap) SAL_OVERRIDE;
    virtual bool            drawAlphaRect( long nX, long nY, long nWidth, long nHeight,
                                           sal_uInt8 nTransparency ) SAL_OVERRIDE;

    virtual SystemGraphicsData GetGraphicsData() const SAL_OVERRIDE;
    virtual bool            SupportsCairo() const SAL_OVERRIDE;
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const SAL_OVERRIDE;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const SAL_OVERRIDE;
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const SAL_OVERRIDE;
    virtual css::uno::Any   GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const SAL_OVERRIDE;

    virtual SystemFontData  GetSysFontData( int nFallbacklevel ) const SAL_OVERRIDE;
};

#endif // INCLUDED_VCL_INC_GENERIC_GENPSPGRAPHICS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
