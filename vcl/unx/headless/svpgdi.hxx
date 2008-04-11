/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svpgdi.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _SVP_SVPGDI_HXX

#include <vcl/salgdi.hxx>
#include <vcl/sallayout.hxx>
#include <basebmp/bitmapdevice.hxx>
#include <basebmp/color.hxx>

class ServerFont;

class SvpSalGraphics : public SalGraphics
{
    basebmp::BitmapDeviceSharedPtr       m_aDevice;
    basebmp::BitmapDeviceSharedPtr       m_aOrigDevice;
    basebmp::BitmapDeviceSharedPtr       m_aClipMap;

    bool                                 m_bUseLineColor;
    basebmp::Color                       m_aLineColor;
    bool                                 m_bUseFillColor;
    basebmp::Color                       m_aFillColor;
    basebmp::Color                       m_aTextColor;

    basebmp::DrawMode                    m_aDrawMode;

    ServerFont*                          m_pServerFont[ MAX_FALLBACK ];
    sal_uInt32                           m_eTextFmt;

protected:
    virtual bool drawAlphaBitmap( const SalTwoRect&, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap );
    virtual bool drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency );

public:
    SvpSalGraphics();
    virtual ~SvpSalGraphics();

    const basebmp::BitmapDeviceSharedPtr& getDevice() const { return m_aDevice; }
    void setDevice( basebmp::BitmapDeviceSharedPtr& rDevice );

    // overload all pure virtual methods
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual void            GetScreenFontResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual USHORT          GetBitCount();
    virtual long            GetGraphicsWidth() const;

    virtual void            ResetClipRegion();
    virtual void            BeginSetClipRegion( ULONG nCount );
    virtual BOOL            unionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void            EndSetClipRegion();

    virtual void            SetLineColor();
    virtual void            SetLineColor( SalColor nSalColor );
    virtual void            SetFillColor();

    virtual void            SetFillColor( SalColor nSalColor );

    virtual void            SetXORMode( BOOL bSet );

    virtual void            SetROPLineColor( SalROPColor nROPColor );
    virtual void            SetROPFillColor( SalROPColor nROPColor );

    virtual void            SetTextColor( SalColor nSalColor );
    virtual USHORT         SetFont( ImplFontSelectData*, int nFallbackLevel );
    virtual void            GetFontMetric( ImplFontMetricData* );
    virtual ULONG           GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs );
    virtual ImplFontCharMap* GetImplFontCharMap() const;
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
};

#endif
