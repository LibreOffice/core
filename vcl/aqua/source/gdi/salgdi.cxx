/*************************************************************************
 *
 *  $RCSfile: salgdi.cxx,v $
 *
 *  $Revision: 1.14 $
 *  last change: $Author: bmahbod $ $Date: 2000-12-01 03:20:53 $
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

#define _SV_SALGDI_CXX

#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_VCLGRAPHICS_H
#include <VCLGraphics.h>
#endif

// =======================================================================

SalGraphics::SalGraphics()
{
    maGraphicsData.mnPenMode        = patCopy;
    maGraphicsData.mbTransparentPen = FALSE;

    maGraphicsData.mhDefBrush         = NULL;
    maGraphicsData.mbTransparentBrush = FALSE;
}

// -----------------------------------------------------------------------

SalGraphics::~SalGraphics()
{
    if ( maGraphicsData.mhClipRgn != NULL )
    {
        DisposeRgn( maGraphicsData.mhClipRgn );
    } // if

    if ( maGraphicsData.mhGrowRgn != NULL )
    {
        DisposeRgn( maGraphicsData.mhGrowRgn );
    } // if

    if ( maGraphicsData.mhDefBrush != NULL )
    {
        DisposePixPat( maGraphicsData.mhDefBrush );
    } // if
}

// -----------------------------------------------------------------------

void SalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
    if ( maGraphicsData.mhWnd ) {
        VCLGraphics_GetScreenResolution( maGraphicsData.mhWnd,
            &rDPIX, &rDPIY );
    }
    else {
        // Stub code: we only support screen resolution right now
        rDPIX = 0;
        rDPIY = 0;
    }
}

// -----------------------------------------------------------------------

void SalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
    if ( maGraphicsData.mhWnd ) {
        VCLGraphics_GetScreenResolution( maGraphicsData.mhWnd,
            &rDPIX, &rDPIY );
    }
    else {
        // Stub code: we only support screen resolution right now
        rDPIX = 0;
        rDPIY = 0;
    }
}

// -----------------------------------------------------------------------

USHORT SalGraphics::GetBitCount()
{
    return 0;
}

// -----------------------------------------------------------------------

void SalGraphics::ResetClipRegion()
{
}

// -----------------------------------------------------------------------

void SalGraphics::BeginSetClipRegion( ULONG nRectCount )
{
}


// -----------------------------------------------------------------------

BOOL SalGraphics::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    return TRUE;
}

// -----------------------------------------------------------------------

void SalGraphics::EndSetClipRegion()
{
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor()
{
    maGraphicsData.mbTransparentPen = TRUE;
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor( SalColor nSalColor )
{
    RGBColor aRGBColor;

    aRGBColor.red   = SALCOLOR_RED   ( nSalColor );
    aRGBColor.green = SALCOLOR_GREEN ( nSalColor );
    aRGBColor.blue  = SALCOLOR_BLUE  ( nSalColor );

    maGraphicsData.mbTransparentPen = FALSE;
    maGraphicsData.maPenColor       = aRGBColor;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor()
{
    maGraphicsData.mbTransparentBrush = TRUE;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor( SalColor nSalColor )
{
    RGBColor  aRGBColor;

    aRGBColor.red   = SALCOLOR_RED   ( nSalColor );
    aRGBColor.green = SALCOLOR_GREEN ( nSalColor );
    aRGBColor.blue  = SALCOLOR_BLUE  ( nSalColor );

    maGraphicsData.maBrushColor       = aRGBColor;
    maGraphicsData.mbTransparentBrush = FALSE;

    if ( maGraphicsData.mhDefBrush != NULL )
    {
        DisposePixPat( maGraphicsData.mhDefBrush );

        maGraphicsData.mhDefBrush = NULL;
    }

    maGraphicsData.mhDefBrush = NewPixPat();

    if (    (    maGraphicsData.mhDefBrush  != NULL )
         && (  *(maGraphicsData.mhDefBrush) != NULL )
       )
    {
        MakeRGBPat( maGraphicsData.mhDefBrush, &aRGBColor );
    } // if
}

// -----------------------------------------------------------------------

void SalGraphics::SetXORMode( BOOL bSet )
{
    if ( bSet == TRUE )
    {
        maGraphicsData.mnPenMode = patXor;
    } // if
    else
    {
        maGraphicsData.mnPenMode = patCopy;
    } // else
}

// -----------------------------------------------------------------------

void SalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
}

// -----------------------------------------------------------------------

void SalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY )
{
    VCLVIEW hView = maGraphicsData.mhDC;

    if ( hView != NULL )
    {
        VCLGraphics_DrawPixel ( hView, nX, nY );
    } // if
} // SalGraphics::DrawPixel

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY, SalColor nSalColor )
{
    VCLVIEW hView = maGraphicsData.mhDC;

    if ( hView != NULL )
    {
        RGBColor aPixelRGBColor;

        aPixelRGBColor.red   = SALCOLOR_RED   ( nSalColor );
        aPixelRGBColor.green = SALCOLOR_GREEN ( nSalColor );
        aPixelRGBColor.blue  = SALCOLOR_BLUE  ( nSalColor );

        VCLGraphics_DrawColorPixel ( hView, nX, nY, &aPixelRGBColor );
    } // if
}

// -----------------------------------------------------------------------

void SalGraphics::DrawLine( long nX1, long nY1, long nX2, long nY2 )
{
    VCLVIEW  hView = maGraphicsData.mhDC;

    if ( hView != NULL )
    {
        if ( maGraphicsData.mbTransparentPen == TRUE )
        {
            VCLGraphics_DrawLine ( hView,
                                   nX1,
                                   nY1,
                                   nX2,
                                   nY2
                                 );
        } // if
        else
        {
            RGBColor  aLineColor = maGraphicsData.maPenColor;

            VCLGraphics_DrawColorLine (  hView,
                                         nX1,
                                         nY1,
                                         nX2,
                                         nY2,
                                        &aLineColor
                                      );
        } // else
    } // if
} // SalGraphics::DrawLine

// -----------------------------------------------------------------------

void SalGraphics::DrawRect( long nX, long nY, long nWidth, long nHeight )
{
    VCLVIEW hView = maGraphicsData.mhDC;

    if ( hView != NULL )
    {
        if ( maGraphicsData.mbTransparentBrush == TRUE )
        {
            VCLGraphics_DrawRect ( hView,
                                   nX,
                           nY,
                           nWidth,
                           nHeight
                         );
        } // if
        else
        {
            RGBColor  aRectFillColor = maGraphicsData.maBrushColor;

            VCLGraphics_DrawColorRect (  hView,
                                         nX,
                                         nY,
                                         nWidth,
                                         nHeight,
                                        &aRectFillColor
                                      );
        } // else
    } // if
} // SalGraphics::DrawRect

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyLine( ULONG nPoints, const SalPoint *pPtAry )
{
    if  ( ( nPoints > 1 ) && ( pPtAry != NULL ) )
    {
        long      i;
        long      pXPtsArray[nPoints];
        long      pYPtsArray[nPoints];
        VCLVIEW   hView = maGraphicsData.mhDC;

        for ( i = 0; i < nPoints; i++ )
        {
            pXPtsArray[i] = pPtAry[i].mnX;
            pYPtsArray[i] = pPtAry[i].mnY;
        } // for

        if ( hView != NULL )
        {
            VCLGraphics_DrawPolygon ( hView,
                                      nPoints,
                                      pXPtsArray,
                                      pYPtsArray
                                    );
        } // if
    } // if
} // SalGraphics::DrawPolyLine

// -----------------------------------------------------------------------

void SalGraphics::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
    if  ( ( nPoints > 1 ) && ( pPtAry != NULL ) )
    {
        long      i;
        long      pXPtsArray[nPoints];
        long      pYPtsArray[nPoints];

        VCLVIEW   hView          = maGraphicsData.mhDC;
        RGBColor  aPolyFillColor = maGraphicsData.maBrushColor;

        for ( i = 0; i < nPoints; i++ )
        {
            pXPtsArray[i] = pPtAry[i].mnX;
            pYPtsArray[i] = pPtAry[i].mnY;
        } // for

        if ( hView != NULL )
        {
            VCLGraphics_DrawColorPolygon (  hView,
                                            nPoints,
                                            pXPtsArray,
                                            pYPtsArray,
                                           &aPolyFillColor
                                         );
        } // if
    } // if
} // SalGraphics::DrawPolygon

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
                                   PCONSTSALPOINT* pPtAry )
{
}

// -----------------------------------------------------------------------

void SalGraphics::CopyBits( const SalTwoRect* pPosAry,
                            SalGraphics* pSrcGraphics )
{
    if ( ( maGraphicsData.mpCGrafPort != NULL ) && ( pPosAry != NULL ) )
    {
        VCLVIEW hView = maGraphicsData.mhDC;

        if ( hView != NULL )
        {
            const BitMap  *pDstBitMap = GetPortBitMapForCopyBits( maGraphicsData.mpCGrafPort );

            if ( pDstBitMap != NULL )
            {
                Rect   aSrcRect;
                Rect   aDstRect;
                short  nMode;

                SetRect( &aSrcRect,
                          pPosAry->mnSrcX,
                          pPosAry->mnSrcY,
                          pPosAry->mnSrcX + pPosAry->mnSrcWidth,
                          pPosAry->mnSrcY + pPosAry->mnSrcHeight
                       );

                SetRect( &aDstRect,
                          pPosAry->mnDestX,
                          pPosAry->mnDestY,
                          pPosAry->mnDestX + pPosAry->mnDestWidth,
                          pPosAry->mnDestY + pPosAry->mnDestHeight
                       );

                // NOTE: it is a good practice to check rectangle bounds before proceeding

                if ( maGraphicsData.mnPenMode == patCopy )
                {
                    nMode = srcCopy;
                } // if
                else
                {
                    nMode = srcXor;
                } // else

                if ( (  pSrcGraphics != NULL ) && ( pSrcGraphics->maGraphicsData.mpCGrafPort != NULL ) )
                {
                    const BitMap  *pSrcBitMap = GetPortBitMapForCopyBits( pSrcGraphics->maGraphicsData.mpCGrafPort );

                    if ( pSrcBitMap != NULL )
                    {
                        VCLGraphics_CopyBits (  hView,
                                                               pSrcBitMap,
                                                                pDstBitMap,
                                                               &aSrcRect,
                                                               &aDstRect,
                                                                nMode,
                                                               NULL
                                                           );
                    } // if
                } // if
                else
                {
                    VCLGraphics_CopyBits (  hView,
                                                            pDstBitMap,
                                                            pDstBitMap,
                                                      &aSrcRect,
                                                           &aDstRect,
                                                       nMode,
                                            NULL
                                                   );
                } // else
            } // if
        } // if
    } // if
} // SalGraphics::CopyBits

// -----------------------------------------------------------------------

void SalGraphics::CopyArea( long nDestX, long nDestY,
                            long nSrcX, long nSrcY,
                            long nSrcWidth, long nSrcHeight,
                            USHORT nFlags )
{
}

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap )
{
}

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap,
                              SalColor nTransparentColor )
{
}

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap,
                              const SalBitmap& rTransparentBitmap )
{
}

// -----------------------------------------------------------------------

void SalGraphics::DrawMask( const SalTwoRect* pPosAry,
                            const SalBitmap& rSalBitmap,
                            SalColor nMaskColor )
{
}

// -----------------------------------------------------------------------

SalBitmap* SalGraphics::GetBitmap( long nX, long nY, long nDX, long nDY )
{
    return NULL;
}

// -----------------------------------------------------------------------

SalColor SalGraphics::GetPixel( long nX, long nY )
{
}

// -----------------------------------------------------------------------

void SalGraphics::Invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
}

// -----------------------------------------------------------------------

void SalGraphics::Invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nSalFlags )
{
}

// -----------------------------------------------------------------------

BOOL SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight,
              void* pPtr, ULONG nSize )
{
    return FALSE;
}

// -----------------------------------------------------------------------

void SalGraphics::SetTextColor( SalColor nSalColor )
{
    RGBColor aRGBColor;

    aRGBColor.red   = SALCOLOR_RED   ( nSalColor );
    aRGBColor.green = SALCOLOR_GREEN ( nSalColor );
    aRGBColor.blue  = SALCOLOR_BLUE  ( nSalColor );

    maGraphicsData.maTextColor = aRGBColor;
}

// -----------------------------------------------------------------------

USHORT SalGraphics::SetFont( ImplFontSelectData* pFont )
{
    return 0;
}

// -----------------------------------------------------------------------

long SalGraphics::GetCharWidth( sal_Unicode nChar1, sal_Unicode nChar2, long* pWidthAry )
{
    // Stub code: we have not yet written any interfaces to native fonts.
    // However, we need to get some font info in order to continue porting.
    return 10;
}

// -----------------------------------------------------------------------

void SalGraphics::GetFontMetric( ImplFontMetricData* pMetric )
{
    // Stub code: we have not yet written any interfaces to native fonts.
    // However, we need to get some font info in order to continue porting.
    pMetric->mnAscent = 10;
    pMetric->mnDescent = 10;
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs )
{
    return 0;
}

// -----------------------------------------------------------------------

void SalGraphics::GetDevFontList( ImplDevFontList* pList )
{
    // Stub code: we have not yet written any interfaces to native fonts.
    // However, we need to create at least one fake font to continue our
    // porting.
    ImplFontData *pFontData = new ImplFontData;
    pFontData->mnWidth = 10;
    pFontData->mnHeight = 10;
    pList->Add( pFontData );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawText( long nX, long nY,
                            const xub_Unicode* pStr, xub_StrLen nLen )
{
}

// -----------------------------------------------------------------------

void SalGraphics::DrawTextArray( long nX, long nY,
                                 const xub_Unicode* pStr, xub_StrLen nLen,
                                 const long* pDXAry )
{
}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphBoundRect( xub_Unicode cChar, long* pX, long* pY,
                                     long* pWidth, long* pHeight )
{
    return FALSE;
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetGlyphOutline( xub_Unicode cChar, USHORT** ppPolySizes,
                                    SalPoint** ppPoints, BYTE** ppFlags )
{
    return 0;
}
