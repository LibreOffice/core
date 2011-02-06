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

#ifndef _VCL_PSPGRAPHICS_H
#define _VCL_PSPGRAPHICS_H


#include "vcl/fontmanager.hxx"
#include "vcl/salgdi.hxx"
#include "vcl/sallayout.hxx"
#include "vcl/dllapi.h"

namespace psp { struct JobData; class PrinterGfx; }

class ServerFont;
class ImplDevFontAttributes;
class SalInfoPrinter;

class VCL_DLLPUBLIC PspGraphics : public SalGraphics
{
    psp::JobData*               m_pJobData;
    psp::PrinterGfx*            m_pPrinterGfx;
    String*                     m_pPhoneNr;
    bool                        m_bSwallowFaxNo;
    String                      m_aPhoneCollection;
    bool                        m_bPhoneCollectionActive;

    ServerFont*                 m_pServerFont[ MAX_FALLBACK ];
    bool                        m_bFontVertical;
    SalInfoPrinter*             m_pInfoPrinter;
public:
    PspGraphics( psp::JobData* pJob, psp::PrinterGfx* pGfx, String* pPhone, bool bSwallow, SalInfoPrinter* pInfoPrinter )
            : m_pJobData( pJob ),
              m_pPrinterGfx( pGfx ),
              m_pPhoneNr( pPhone ),
              m_bSwallowFaxNo( bSwallow ),
              m_bPhoneCollectionActive( false ),
              m_bFontVertical( false ),
              m_pInfoPrinter( pInfoPrinter )
    { for( int i = 0; i < MAX_FALLBACK; i++ ) m_pServerFont[i] = 0; }
    virtual ~PspGraphics();

    // helper methods for sharing with X11SalGraphics
    static const void* DoGetEmbedFontData( psp::fontID aFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen );
    static void DoFreeEmbedFontData( const void* pData, long nLen );
    static const Ucs2SIntMap* DoGetFontEncodingVector( psp::fontID aFont, const Ucs2OStrMap** pNonEncoded );
    static void DoGetGlyphWidths( psp::fontID aFont,
                                  bool bVertical,
                                  Int32Vector& rWidths,
                                  Ucs2UIntMap& rUnicodeEnc );

    static ImplDevFontAttributes Info2DevFontAttributes( const psp::FastPrintFontInfo& );
    static void AnnounceFonts( ImplDevFontList*, const psp::FastPrintFontInfo& );
    static FontWidth    ToFontWidth (psp::width::type eWidth);
    static FontWeight   ToFontWeight (psp::weight::type eWeight);
    static FontPitch    ToFontPitch (psp::pitch::type ePitch);
    static FontItalic   ToFontItalic (psp::italic::type eItalic);
    static FontFamily   ToFontFamily (psp::family::type eFamily);

    // overload all pure virtual methods
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual USHORT          GetBitCount() const;
    virtual long            GetGraphicsWidth() const;

    virtual void            ResetClipRegion();
    virtual void            BeginSetClipRegion( ULONG nCount );
    virtual BOOL            unionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual bool            unionClipRegion( const ::basegfx::B2DPolyPolygon& );
    virtual void            EndSetClipRegion();

    virtual void            SetLineColor();
    virtual void            SetLineColor( SalColor nSalColor );
    virtual void            SetFillColor();
    virtual void            SetFillColor( SalColor nSalColor );
    virtual void            SetXORMode( bool bSet, bool );
    virtual void            SetROPLineColor( SalROPColor nROPColor );
    virtual void            SetROPFillColor( SalROPColor nROPColor );

    virtual void            SetTextColor( SalColor nSalColor );
    virtual USHORT          SetFont( ImplFontSelectData*, int nFallbackLevel );
    virtual void            GetFontMetric( ImplFontMetricData*, int nFallbackLevel );
    virtual ULONG           GetKernPairs( ULONG nMaxPairs, ImplKernPairData* );
    virtual ImplFontCharMap* GetImplFontCharMap() const;
    virtual bool GetImplFontLayoutCapabilities(FontLayoutCapabilities &rGetImplFontLayoutCapabilities) const;
    virtual void            GetDevFontList( ImplDevFontList* );
    virtual void            GetDevFontSubstList( OutputDevice* );
    virtual bool            AddTempDevFont( ImplDevFontList*, const String& rFileURL, const String& rFontName );
    virtual BOOL            CreateFontSubset( const rtl::OUString& rToFile,
                                              const ImplFontData*,
                                              sal_Int32* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo
                                              );
    virtual const Ucs2SIntMap* GetFontEncodingVector( const ImplFontData*, const Ucs2OStrMap** ppNonEncoded );
    virtual const void* GetEmbedFontData( const ImplFontData*,
                                          const sal_Ucs* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen );
    virtual void            GetGlyphWidths( const ImplFontData*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc );
    virtual BOOL            GetGlyphBoundRect( long nIndex, Rectangle& );
    virtual BOOL            GetGlyphOutline( long nIndex, ::basegfx::B2DPolyPolygon& );
    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void            DrawServerFontLayout( const ServerFontLayout& );
    virtual bool            supportsOperation( OutDevSupportType ) const;
    virtual void            drawPixel( long nX, long nY );
    virtual void            drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void            drawPolyLine( ULONG nPoints, const SalPoint* pPtAry );
    virtual void            drawPolygon( ULONG nPoints, const SalPoint* pPtAry );
    virtual void            drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry );
    virtual bool            drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency );
    virtual bool            drawPolyLine( const basegfx::B2DPolygon&, double fTransparency, const basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin);
    virtual sal_Bool        drawPolyLineBezier( ULONG nPoints,
                                                const SalPoint* pPtAry,
                                                const BYTE* pFlgAry );
    virtual sal_Bool        drawPolygonBezier( ULONG nPoints,
                                               const SalPoint* pPtAry,
                                               const BYTE* pFlgAry );
    virtual sal_Bool        drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const BYTE* const* pFlgAry );
    virtual void            copyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      USHORT nFlags );
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
    virtual void            invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual BOOL            drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize );
    virtual bool            filterText( const String& rOrigText, String& rNewText, xub_StrLen nIndex, xub_StrLen& rLen, xub_StrLen& rCutStart, xub_StrLen& rCutStop );

    virtual bool            drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap );
    virtual bool            drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency );

    virtual SystemGraphicsData GetGraphicsData() const;
    virtual SystemFontData     GetSysFontData( int nFallbacklevel ) const;
};

#endif // _VCL_PSPGRAPHICS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
