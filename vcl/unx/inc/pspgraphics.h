/*************************************************************************
 *
 *  $RCSfile: pspgraphics.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:33:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _VCL_PSPGRAPHICS_H
#define _VCL_PSPGRAPHICS_H


#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _PSPRINT_FONTMANAGER_HXX
#include <psprint/fontmanager.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif

#ifndef _USE_PRINT_EXTENSION_
namespace psp { struct JobData; class PrinterGfx; }
#endif

class ServerFont;
class ImplDevFontAttributes;

class PspGraphics : public SalGraphics
{
    psp::JobData*               m_pJobData;
    psp::PrinterGfx*            m_pPrinterGfx;
    String*                     m_pPhoneNr;
    bool                        m_bSwallowFaxNo;

    ServerFont*                 m_pServerFont[ MAX_FALLBACK ];
    bool                        m_bFontVertical;
public:
    PspGraphics( psp::JobData* pJob, psp::PrinterGfx* pGfx, String* pPhone = NULL, bool bSwallow = false )
            : m_pJobData( pJob ),
              m_pPrinterGfx( pGfx ),
              m_pPhoneNr( pPhone ),
              m_bSwallowFaxNo( bSwallow ),
              m_bFontVertical( false )
    { for( int i = 0; i < MAX_FALLBACK; i++ ) m_pServerFont[i] = 0; }
    virtual ~PspGraphics();

    String            FaxPhoneComment( const String& rOrig, xub_StrLen nIndex, xub_StrLen& rLen, xub_StrLen& rCutStart, xub_StrLen& rCutStop ) const;

    // helper methods for sharing with X11SalGraphics
    static bool DoCreateFontSubset( const rtl::OUString& rToFile,
                                    psp::fontID aFont,
                                    sal_Int32* pGlyphIDs,
                                    sal_uInt8* pEncoding,
                                    sal_Int32* pWidths,
                                    int nGlyphs,
                                    FontSubsetInfo& rInfo );
    static const void* DoGetEmbedFontData( psp::fontID aFont, const sal_Unicode* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen );
    static void DoFreeEmbedFontData( const void* pData, long nLen );
    static const std::map< sal_Unicode, sal_Int32 >* DoGetFontEncodingVector( psp::fontID aFont, const std::map< sal_Unicode, rtl::OString >** pNonEncoded );
    static ImplDevFontAttributes Info2DevFontAttributes( const psp::FastPrintFontInfo& );
    static void AnnounceFonts( ImplDevFontList*, const psp::FastPrintFontInfo& );
    static FontWidth    ToFontWidth (psp::width::type eWidth);
    static FontWeight   ToFontWeight (psp::weight::type eWeight);
    static FontPitch    ToFontPitch (psp::pitch::type ePitch);
    static FontItalic   ToFontItalic (psp::italic::type eItalic);
    static FontFamily   ToFontFamily (psp::family::type eFamily);

    // overload all pure virtual methods
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual void            GetScreenFontResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual USHORT          GetBitCount();
    virtual long            GetGraphicsWidth();

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
    virtual USHORT          SetFont( ImplFontSelectData*, int nFallbackLevel );
    virtual void            GetFontMetric( ImplFontMetricData* );
    virtual ULONG           GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs );
    virtual ImplFontCharMap* GetImplFontCharMap() const;
    virtual void            GetDevFontList( ImplDevFontList* );
    virtual void            GetDevFontSubstList( OutputDevice* );
    virtual bool            AddTempDevFont( ImplDevFontList*, const String& rFileURL, const String& rFontName );
    virtual BOOL            CreateFontSubset( const rtl::OUString& rToFile,
                                              ImplFontData* pFont,
                                              sal_Int32* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo
                                              );
    virtual const std::map< sal_Unicode, sal_Int32 >* GetFontEncodingVector( ImplFontData* pFont, const std::map< sal_Unicode, rtl::OString >** ppNonEncoded );
    virtual const void* GetEmbedFontData( ImplFontData* pFont,
                                          const sal_Unicode* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen );
    virtual BOOL            GetGlyphBoundRect( long nIndex, Rectangle& );
    virtual BOOL            GetGlyphOutline( long nIndex, PolyPolygon& );
    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void            DrawServerFontLayout( const ServerFontLayout& );
    virtual void            drawPixel( long nX, long nY );
    virtual void            drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void            drawPolyLine( ULONG nPoints, const SalPoint* pPtAry );
    virtual void            drawPolygon( ULONG nPoints, const SalPoint* pPtAry );
    virtual void            drawPolyPolygon( ULONG nPoly,
                                             const ULONG* pPoints,
                                             PCONSTSALPOINT* pPtAry );
    virtual sal_Bool        drawPolyLineBezier( ULONG nPoints,
                                                const SalPoint* pPtAry,
                                                const BYTE* pFlgAry );
    virtual sal_Bool        drawPolygonBezier( ULONG nPoints,
                                               const SalPoint* pPtAry,
                                               const BYTE* pFlgAry );
    virtual sal_Bool        drawPolyPolygonBezier( ULONG nPoly,
                                                   const ULONG* pPoints,
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

#endif // _VCL_PSPGRAPHICS_H
