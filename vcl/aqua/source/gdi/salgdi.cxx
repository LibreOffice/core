/*************************************************************************
 *
 *  $RCSfile: salgdi.cxx,v $
 *
 *  $Revision: 1.16 $
 *  last change: $Author: bmahbod $ $Date: 2000-12-05 01:57:55 $
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
    RGBColor aRGBBackColor;
    RGBColor aRGBForeColor;

    maGraphicsData.mnPenMode        = patCopy;
    maGraphicsData.mbTransparentPen = FALSE;

    maGraphicsData.mhDefBrush         = NULL;
    maGraphicsData.mbTransparentBrush = FALSE;

    // White color

    aRGBBackColor.red   = 0xffff;
    aRGBBackColor.green = 0xffff;
    aRGBBackColor.blue  = 0xffff;

    // Black color

    aRGBForeColor.red   = 0x0000;
    aRGBForeColor.green = 0x0000;
    aRGBForeColor.blue  = 0x0000;

    // Set pen and text colors

    maGraphicsData.maPenColor   = aRGBForeColor;
    maGraphicsData.maBrushColor = aRGBForeColor;
    maGraphicsData.maTextColor  = aRGBForeColor;

    // Set background and foreground colors

    RGBBackColor( &aRGBBackColor );
    RGBForeColor( &aRGBForeColor );
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

// =======================================================================

// =======================================================================

static void CheckRects ( Rect *rSrcRect, Rect *rDstRect, const Rect *rPortBoundsRect )

{
    if ( rSrcRect->top < rPortBoundsRect->top )
    {

        rDstRect->top += (rPortBoundsRect->top - rSrcRect->top);
        rSrcRect->top  = rPortBoundsRect->top;
    } // if

    if ( rSrcRect->left < rPortBoundsRect->left )
    {
        rDstRect->left += (rPortBoundsRect->left - rSrcRect->left);
        rSrcRect->left  = rPortBoundsRect->left;
    } // if

    if ( rSrcRect->bottom > rPortBoundsRect->bottom )
    {

        rDstRect->bottom += (rPortBoundsRect->bottom - rSrcRect->bottom);
        rSrcRect->bottom  = rPortBoundsRect->bottom;
    } // if

    if ( rSrcRect->right > rPortBoundsRect->right )
    {
        rDstRect->right += (rPortBoundsRect->right - rSrcRect->right);
        rSrcRect->right  = rPortBoundsRect->right;
    } // if
}

// -----------------------------------------------------------------------

static RGBColor SALColor2RGBColor ( SalColor nSalColor )
{
    const unsigned long aRGBMask[3] = { 0x00FF0000, 0x0000FF00, 0x000000FF };

    RGBColor aRGBColor;

    // Converting from their SAL color scheme causes only colors that are
    // close to black to be displayed on-screen.

    // Temp:  for now, just give me shades of green

    aRGBColor.red   = (unsigned short)( nSalColor & aRGBMask[0] );
    aRGBColor.green = (unsigned short)( nSalColor & aRGBMask[1] );
    aRGBColor.blue  = (unsigned short)( nSalColor & aRGBMask[2] );

    return aRGBColor;
}

// =======================================================================

// =======================================================================

void SalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
    if ( maGraphicsData.mhWnd )
    {
        VCLGraphics_GetScreenResolution( maGraphicsData.mhWnd,
            &rDPIX, &rDPIY );
    }
    else
    {
        // Stub code: we only support screen resolution right now
        rDPIX = 0;
        rDPIY = 0;
    }
}

// -----------------------------------------------------------------------

void SalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
    if ( maGraphicsData.mhWnd )
    {
        VCLGraphics_GetScreenResolution( maGraphicsData.mhWnd,
            &rDPIX, &rDPIY );
    }
    else
    {
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
    maGraphicsData.mbTransparentPen = FALSE;
    maGraphicsData.maPenColor       = SALColor2RGBColor( nSalColor );
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor()
{
    maGraphicsData.mbTransparentBrush = TRUE;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor( SalColor nSalColor )
{
    RGBColor aRGBColor;

    aRGBColor = SALColor2RGBColor( nSalColor );

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

        aPixelRGBColor = SALColor2RGBColor( nSalColor );

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
                Rect   aPortBoundsRect;
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

                GetPortBounds( pSrcGraphics->maGraphicsData.mpCGrafPort, &aPortBoundsRect );

                CheckRects( &aSrcRect, &aDstRect, &aPortBoundsRect );

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
} // SalGraphics::DrawMask

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
    maGraphicsData.maTextColor = SALColor2RGBColor( nSalColor );
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

// =======================================================================

