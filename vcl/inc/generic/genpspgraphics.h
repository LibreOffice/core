/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _VCL_GENPSPGRAPHICS_H
#define _VCL_GENPSPGRAPHICS_H


#include "vcl/fontmanager.hxx"

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
    rtl::OUString*                     m_pPhoneNr;
    bool                        m_bSwallowFaxNo;
    rtl::OUString                      m_aPhoneCollection;
    bool                        m_bPhoneCollectionActive;

    ServerFont*                 m_pServerFont[ MAX_FALLBACK ];
    bool                        m_bFontVertical;
    SalInfoPrinter*             m_pInfoPrinter;
public:
            GenPspGraphics();
    virtual ~GenPspGraphics();

    void  Init( psp::JobData* pJob, psp::PrinterGfx* pGfx,
                rtl::OUString* pPhone, bool bSwallow,
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
    virtual sal_uLong           GetKernPairs( sal_uLong nMaxPairs, ImplKernPairData* );
    virtual const ImplFontCharMap* GetImplFontCharMap() const;
    virtual bool GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const;
    virtual void            GetDevFontList( ImplDevFontList* );
    // graphics must drop any cached font info
    virtual void ClearDevFontCache();
    virtual void            GetDevFontSubstList( OutputDevice* );
    virtual bool            AddTempDevFont( ImplDevFontList*, const rtl::OUString& rFileURL, const rtl::OUString& rFontName );

    virtual sal_Bool            CreateFontSubset( const rtl::OUString& rToFile,
                                              const PhysicalFontFace*,
                                              sal_Int32* pGlyphIDs,
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
    virtual sal_Bool            GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& );
    virtual sal_Bool            GetGlyphOutline( sal_GlyphId nIndex, ::basegfx::B2DPolyPolygon& );
    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void            DrawServerFontLayout( const ServerFontLayout& );
    virtual bool            supportsOperation( OutDevSupportType ) const;
    virtual void            drawPixel( long nX, long nY );
    virtual void            drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void            drawPolyLine( sal_uIntPtr nPoints, const SalPoint* pPtAry );
    virtual void            drawPolygon( sal_uIntPtr nPoints, const SalPoint* pPtAry );
    virtual void            drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry );
    virtual bool            drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency );
    virtual bool            drawPolyLine( const basegfx::B2DPolygon&, double fTransparency, const basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin);
    virtual sal_Bool        drawPolyLineBezier( sal_uIntPtr nPoints,
                                                const SalPoint* pPtAry,
                                                const sal_uInt8* pFlgAry );
    virtual sal_Bool        drawPolygonBezier( sal_uIntPtr nPoints,
                                               const SalPoint* pPtAry,
                                               const sal_uInt8* pFlgAry );
    virtual sal_Bool        drawPolyPolygonBezier( sal_uInt32 nPoly,
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
    virtual void            copyBits( const SalTwoRect* pPosAry,
                                      SalGraphics* pSrcGraphics );
    virtual void            drawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap );
    virtual void            drawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nTransparentColor );
    virtual void            drawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rTransparentBitmap );
    virtual void            drawMask( const SalTwoRect* pPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor );
    virtual SalBitmap*      getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor        getPixel( long nX, long nY );
    virtual void            invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags );
    virtual void            invert( sal_uIntPtr nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual sal_Bool            drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uIntPtr nSize );
    virtual bool            filterText( const rtl::OUString& rOrigText, rtl::OUString& rNewText, xub_StrLen nIndex, xub_StrLen& rLen, xub_StrLen& rCutStart, xub_StrLen& rCutStop );

    virtual bool            drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap );
    virtual bool            drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency );

    virtual SystemGraphicsData GetGraphicsData() const;
    virtual SystemFontData     GetSysFontData( int nFallbacklevel ) const;
};

#endif // _VCL_GENPSPGRAPHICS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
