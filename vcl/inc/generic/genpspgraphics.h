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


#include "fontmanager.hxx"

#include "vclpluginapi.h"
#include "salgdi.hxx"
#include "vcl/vclenum.hxx"
#include <sallayout.hxx>

namespace psp { struct JobData; class PrinterGfx; }

class ServerFont;
class ImplDevFontAttributes;
class SalInfoPrinter;

class VCL_DLLPUBLIC GenPspGraphics : public SalGraphics
{
 protected:
    psp::JobData*               m_pJobData;
    psp::PrinterGfx*            m_pPrinterGfx;

    ServerFont*                 m_pServerFont[ MAX_FALLBACK ];
    bool                        m_bFontVertical;
    SalInfoPrinter*             m_pInfoPrinter;
public:
            GenPspGraphics();
    virtual ~GenPspGraphics();

    void  Init( psp::JobData* pJob, psp::PrinterGfx* pGfx,
                SalInfoPrinter* pInfoPrinter );

    // helper methods
    static const void *     DoGetEmbedFontData ( psp::fontID aFont, const sal_Ucs* pUnicodes,
                                                 sal_Int32* pWidths, FontSubsetInfo& rInfo,
                                                 long* pDataLen );
    static void             DoFreeEmbedFontData( const void* pData, long nLen );

    // helper methods for sharing with X11SalGraphics
    static const Ucs2SIntMap* DoGetFontEncodingVector( psp::fontID aFont, const Ucs2OStrMap** pNonEncoded );
    static void DoGetGlyphWidths( psp::fontID aFont,
                                  bool bVertical,
                                  Int32Vector& rWidths,
                                  Ucs2UIntMap& rUnicodeEnc );

    static ImplDevFontAttributes Info2DevFontAttributes( const psp::FastPrintFontInfo& );
    static void AnnounceFonts( ImplDevFontList*, const psp::FastPrintFontInfo& );

    // overload all pure virtual methods
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual sal_uInt16          GetBitCount() const;
    virtual long            GetGraphicsWidth() const;

    virtual void            ResetClipRegion();
    virtual bool            setClipRegion( const Region& );

    virtual void            SetLineColor();
    virtual void            SetLineColor( SalColor nSalColor );
    virtual void            SetFillColor();
    virtual void            SetFillColor( SalColor nSalColor );
    virtual void            SetXORMode( bool bSet, bool );
    virtual void            SetROPLineColor( SalROPColor nROPColor );
    virtual void            SetROPFillColor( SalROPColor nROPColor );

    virtual void            SetTextColor( SalColor nSalColor );
    virtual sal_uInt16          SetFont( FontSelectPattern*, int nFallbackLevel );
    virtual void            GetFontMetric( ImplFontMetricData*, int nFallbackLevel );
    virtual const ImplFontCharMap* GetImplFontCharMap() const;
    virtual bool GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const;
    virtual void            GetDevFontList( ImplDevFontList* );
    // graphics must drop any cached font info
    virtual void ClearDevFontCache();
    virtual bool            AddTempDevFont( ImplDevFontList*, const OUString& rFileURL, const OUString& rFontName );

    virtual bool            CreateFontSubset( const OUString& rToFile,
                                              const PhysicalFontFace*,
                                              sal_GlyphId* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo
                                              );
    virtual const Ucs2SIntMap* GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded );
    virtual const void*     GetEmbedFontData( const PhysicalFontFace*,
                                              const sal_Ucs* pUnicodes,
                                              sal_Int32* pWidths,
                                              FontSubsetInfo& rInfo,
                                              long* pDataLen );
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen );
    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc );
    virtual bool            GetGlyphBoundRect( sal_GlyphId, Rectangle& );
    virtual bool            GetGlyphOutline( sal_GlyphId, ::basegfx::B2DPolyPolygon& );
    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void            DrawServerFontLayout( const ServerFontLayout& );
    virtual bool            supportsOperation( OutDevSupportType ) const;
    virtual void            drawPixel( long nX, long nY );
    virtual void            drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void            drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry );
    virtual void            drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry );
    virtual void            drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry );
    virtual bool            drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency );
    virtual bool            drawPolyLine(
        const basegfx::B2DPolygon&,
        double fTransparency,
        const basegfx::B2DVector& rLineWidths,
        basegfx::B2DLineJoin,
        com::sun::star::drawing::LineCap);
    virtual bool        drawPolyLineBezier( sal_uInt32 nPoints,
                                                const SalPoint* pPtAry,
                                                const sal_uInt8* pFlgAry );
    virtual bool        drawPolygonBezier( sal_uInt32 nPoints,
                                               const SalPoint* pPtAry,
                                               const sal_uInt8* pFlgAry );
    virtual bool        drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const sal_uInt8* const* pFlgAry );
    virtual void            copyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      sal_uInt16 nFlags );
    virtual void            copyBits( const SalTwoRect& rPosAry,
                                      SalGraphics* pSrcGraphics );
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap );
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nTransparentColor );
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rTransparentBitmap );
    virtual void            drawMask( const SalTwoRect& rPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor );
    virtual SalBitmap*      getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor        getPixel( long nX, long nY );
    virtual void            invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags );
    virtual void            invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual bool            drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uIntPtr nSize );
    virtual bool            drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap );
    virtual bool drawTransformedBitmap(
        const basegfx::B2DPoint& rNull,
        const basegfx::B2DPoint& rX,
        const basegfx::B2DPoint& rY,
        const SalBitmap& rSourceBitmap,
        const SalBitmap* pAlphaBitmap);
    virtual bool            drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency );

    virtual SystemGraphicsData GetGraphicsData() const;
    virtual SystemFontData     GetSysFontData( int nFallbacklevel ) const;
};

#endif // INCLUDED_VCL_INC_GENERIC_GENPSPGRAPHICS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
