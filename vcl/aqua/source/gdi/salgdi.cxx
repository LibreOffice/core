/*************************************************************************
 *
 *  $RCSfile: salgdi.cxx,v $
 *
 *  $Revision: 1.52 $
 *  last change: $Author: bmahbod $ $Date: 2001-02-21 20:48:46 $
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

#ifndef _SV_SALCONST_H
#include <salconst.h>
#endif

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

#ifndef _SV_SALCOLORUTILS_HXX
#include <salcolorutils.hxx>
#endif

#ifndef _SV_SALRECTANGLEUTILS_HXX
#include <salrectangleutils.hxx>
#endif

#ifndef _SV_SALGDIUTILS_HXX
#include <salgdiutils.hxx>
#endif

// =======================================================================

// =======================================================================

SalGraphics::SalGraphics()
{
    // QuickDraw graph port, offscreen graphic world, and graphic device handle

    InitQD( &maGraphicsData );

    // GWorld attributes

    InitGWorld( &maGraphicsData );

    // Regions within a current port

    InitRegions( &maGraphicsData );

    // Font attributes

    InitFont( &maGraphicsData );

    // Pen attributes and status

    InitPen( &maGraphicsData );

    // Brush attributes and status

    InitBrush( &maGraphicsData );

    // Miscellaneous status flags

    InitStatusFlags( &maGraphicsData );

} // SalGraphics Class Constructor

// -----------------------------------------------------------------------

SalGraphics::~SalGraphics()
{
    // Release memory taken up by clip region, off-screen
    // graph world, and colour graph port

    if (    (    ( maGraphicsData.mbGWorldPixelsCopy == TRUE )
              || ( maGraphicsData.mbGWorldPixelsNew  == TRUE )
            )
         && ( maGraphicsData.mhGWorldPixMap != NULL )
           )
    {
        DisposePixMap( maGraphicsData.mhGWorldPixMap );

        maGraphicsData.mhGWorldPixMap = NULL;
    } // if

    if ( maGraphicsData.mhClipRgn != NULL )
    {
        DisposeRgn( maGraphicsData.mhClipRgn );

        maGraphicsData.mhClipRgn = NULL;
    } // if

    if ( maGraphicsData.mpGWorld != NULL )
    {
        DisposeGWorld( maGraphicsData.mpGWorld );

        maGraphicsData.mpGWorld = NULL;
    } // if

    if ( maGraphicsData.mpCGrafPort != NULL )
    {
        DisposeGWorld( maGraphicsData.mpCGrafPort );

        maGraphicsData.mpCGrafPort = NULL;
    } // if

    // Initialize the rest of the fields to zero

    memset( &maGraphicsData, 0, sizeof(SalGraphicsData) );
} // SalGraphics Class Destructor

// =======================================================================

// =======================================================================

void SalGraphics::GetResolution( long& rDPIX,
                                 long& rDPIY
                               )
{
    long nHRes = 0;
    long nVRes = 0;

    maGraphicsData.mnOSStatus = GetGDeviceResolution( &nHRes, &nVRes );

    if ( maGraphicsData.mnOSStatus == noErr )
    {
        rDPIX = nHRes;
        rDPIY = nVRes;
    } // if
} // SalGraphics::GetResolution

// -----------------------------------------------------------------------

void SalGraphics::GetScreenFontResolution( long& rDPIX,
                                           long& rDPIY
                                         )
{
    long nHRes = 0;
    long nVRes = 0;

    maGraphicsData.mnOSStatus = GetGDeviceResolution( &nHRes, &nVRes );

    if ( maGraphicsData.mnOSStatus == noErr )
    {
        rDPIX = nHRes;
        rDPIY = nVRes;
    } // if
} // SalGraphics::GetScreenFontResolution

// -----------------------------------------------------------------------

USHORT SalGraphics::GetBitCount()
{
    unsigned short nBitDepth = 0;

    maGraphicsData.mnOSStatus = GetGDeviceBitDepth( &nBitDepth );

    return nBitDepth;
} // SalGraphics::GetBitCount

// -----------------------------------------------------------------------

void SalGraphics::ResetClipRegion()
{
    if ( !maGraphicsData.mbWindow )
    {
        if ( maGraphicsData.mhClipRgn != NULL )
        {
            DisposeRgn( maGraphicsData.mhClipRgn );

            maGraphicsData.mhClipRgn = NULL;
        } // if
    } // if
    else
    {
        Rect   aClipRect;
        short  nLeft   = 0;
        short  nTop    = 0;
        short  nRight  = 0;
        short  nBottom = 0;

        if ( maGraphicsData.mhClipRgn == NULL )
        {
            maGraphicsData.mhClipRgn = NewRgn();
        } // if

        GetPortBounds( maGraphicsData.mpCGrafPort, &aClipRect);

        nRight  = aClipRect.right  - aClipRect.left;
        nBottom = aClipRect.bottom - aClipRect.top;

        MacSetRectRgn ( maGraphicsData.mhClipRgn,
                        nLeft,
                        nTop,
                        nRight,
                        nBottom
                      );
    } // else

    maGraphicsData.mbClipRgnChanged = TRUE;
} // SalGraphics::ResetClipRegion

// -----------------------------------------------------------------------

void SalGraphics::BeginSetClipRegion( ULONG nRectCount )
{
    #pragma unused(nRectCount)

    if ( maGraphicsData.mhClipRgn != NULL )
    {
        DisposeRgn( maGraphicsData.mhClipRgn );

        maGraphicsData.mhClipRgn = NULL;
    } // if
} // SalGraphics::BeginSetClipRegion

// -----------------------------------------------------------------------

BOOL SalGraphics::UnionClipRegion( long nX,
                                   long nY,
                                   long nWidth,
                                   long nHeight
                                 )
{
    RgnHandle  hClipRegion        = NULL;
    BOOL       bClipRegionsJoined = FALSE;

    hClipRegion = NewRgn();

    if ( hClipRegion != NULL )
    {
        short  nLeft   = nX;
        short  nTop    = nY;
        short  nRight  = nX+nWidth;
        short  nBottom = nY+nHeight;

        MacSetRectRgn ( hClipRegion,
                        nLeft,
                        nTop,
                        nRight,
                        nBottom
                      );

        if ( maGraphicsData.mhClipRgn != NULL )
        {
            MacUnionRgn ( maGraphicsData.mhClipRgn,
                          hClipRegion,
                          maGraphicsData.mhClipRgn
                        );

            maGraphicsData.mnOSStatus = QDErr();

            DisposeRgn( hClipRegion );
        } // if
        else
        {
            maGraphicsData.mhClipRgn = hClipRegion;
        } // else

        if ( maGraphicsData.mnOSStatus == noErr )
        {
            bClipRegionsJoined = TRUE;
        } // if
    } // if

    return bClipRegionsJoined;
} // SalGraphics::UnionClipRegion

// -----------------------------------------------------------------------

void SalGraphics::EndSetClipRegion()
{
    maGraphicsData.mbClipRgnChanged = TRUE;

    if ( maGraphicsData.mhClipRgn == NULL )
    {
        ResetClipRegion();
    }
} // SalGraphics::EndSetClipRegion

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor()
{
    maGraphicsData.mbPenTransparent = TRUE;
} // SalGraphics::SetLineColor

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor( SalColor nSalColor )
{
    maGraphicsData.mbPenTransparent = FALSE;
    maGraphicsData.maPenColor       = SALColor2RGBColor( nSalColor );
} // SalGraphics::SetLineColor

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor()
{
    maGraphicsData.mbBrushTransparent = TRUE;
} // SalGraphics::SetFillColor

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor( SalColor nSalColor )
{
    RGBColor aRGBColor;

    aRGBColor = SALColor2RGBColor( nSalColor );

    maGraphicsData.maBrushColor       = aRGBColor;
    maGraphicsData.mbBrushTransparent = FALSE;
} // SalGraphics::SetFillColor

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
} // SalGraphics::SetXORMode

// -----------------------------------------------------------------------

void SalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    SalColor nSalColor = GetROPSalColor( nROPColor );

    SetLineColor( nSalColor );
} // SalGraphics::SetROPLineColor

// -----------------------------------------------------------------------

void SalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    SalColor nSalColor = GetROPSalColor( nROPColor );

    SetFillColor( nSalColor );
} // SalGraphics::SetROPFillColor

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX,
                             long nY
                           )
{
    if ( BeginGraphics( &maGraphicsData ) )
    {
        RGBColor aPixelRGBColor =  maGraphicsData.maPenColor;

        SetCPixel( nX, nY, &aPixelRGBColor );

        EndGraphics( &maGraphicsData );
    } // if
} // SalGraphics::DrawPixel

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long      nX,
                             long      nY,
                             SalColor  nSalColor
                           )
{
    if ( BeginGraphics( &maGraphicsData ) )
    {
        RGBColor aPixelRGBColor;

        aPixelRGBColor = SALColor2RGBColor( nSalColor );

        SetCPixel( nX, nY, &aPixelRGBColor );

        EndGraphics( &maGraphicsData );
    } // if
} // SalGraphics::DrawPixel

// -----------------------------------------------------------------------

void SalGraphics::DrawLine( long nX1,
                            long nY1,
                            long nX2,
                            long nY2
                          )
{
    if ( BeginGraphics( &maGraphicsData ) )
    {
        MoveTo( nX1, nY1 );

        if ( maGraphicsData.mbPenTransparent == TRUE )
        {
            SetBlackForeColor();
        } // if
        else
        {
            RGBColor  aPenColor = maGraphicsData.maPenColor;

            RGBForeColor( &aPenColor );
        } // else

        MacLineTo( nX2, nY2 );

        EndGraphics( &maGraphicsData );
    } // if
} // SalGraphics::DrawLine

// -----------------------------------------------------------------------

void SalGraphics::DrawRect( long  nX,
                            long  nY,
                            long  nWidth,
                            long  nHeight
                          )
{
    if ( BeginGraphics( &maGraphicsData ) )
    {
        long      nEndX       = 0;
        long      nEndY       = 0;
        RGBColor  aBrushColor = maGraphicsData.maBrushColor;
        Rect      aRect;

        // Compute the second set of (nX,nY) coordinates

        nEndX = nX + nWidth;
        nEndY = nY + nHeight;

        RGBForeColor( &aBrushColor );

        MoveTo( nX, nY );

        MacSetRect( &aRect, nX, nY, nEndX, nEndY );

        if ( maGraphicsData.mbBrushTransparent == TRUE )
        {
            MacFrameRect( &aRect );
        } // if
        else
        {
            PaintRect( &aRect );
        } // else

        EndGraphics( &maGraphicsData );
    } // if
} // SalGraphics::DrawRect

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyLine( ULONG           nPoints,
                                const SalPoint *pPtAry
                              )
{
    if  ( ( nPoints > 1 ) && ( pPtAry != NULL ) )
    {
        if ( BeginGraphics( &maGraphicsData ) )
        {
            long        nPolyEdges = 0;
            PolyHandle  hPolygon   = NULL;

            SetBlackForeColor();

            // Construct a polygon

            hPolygon = OpenPoly();

                if ( hPolygon != NULL )
                {
                    MoveTo( pPtAry[0].mnX,  pPtAry[0].mnY );

                    for ( nPolyEdges = 1;
                          nPolyEdges < nPoints;
                          nPolyEdges++
                        )
                    {
                        MacLineTo( pPtAry[nPolyEdges].mnX,
                                   pPtAry[nPolyEdges].mnY
                                 );
                    } // for

                    MacLineTo( pPtAry[0].mnX,
                               pPtAry[0].mnY
                             );
                } // if

            ClosePoly();

            // Did a QD error occur whilst constructing a polygon?

            maGraphicsData.mnOSStatus = QDErr();

            if (    ( maGraphicsData.mnOSStatus == noErr )
                 && ( hPolygon != NULL )
               )
            {
                FramePoly( hPolygon );

                KillPoly( hPolygon );
            } // if

            EndGraphics( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::DrawPolyLine

// -----------------------------------------------------------------------

void SalGraphics::DrawPolygon( ULONG            nPoints,
                               const SalPoint  *pPtAry
                             )
{
    if  ( ( nPoints > 1 ) && ( pPtAry != NULL ) )
    {
        if ( BeginGraphics( &maGraphicsData ) )
        {
            long        nPolyEdges = 0;
            PolyHandle  hPolygon   = NULL;
            RGBColor    aPolyColor = maGraphicsData.maBrushColor;

            RGBForeColor( &aPolyColor );

            // Construct a polygon

            hPolygon = OpenPoly();

                if ( hPolygon != NULL )
                {
                    MoveTo( pPtAry[0].mnX,  pPtAry[0].mnY );

                    for ( nPolyEdges = 1;
                          nPolyEdges < nPoints;
                          nPolyEdges++
                        )
                    {
                        MacLineTo( pPtAry[nPolyEdges].mnX,
                                   pPtAry[nPolyEdges].mnY
                                 );
                    } // for

                    MacLineTo( pPtAry[0].mnX,
                               pPtAry[0].mnY
                             );
                } // if

            ClosePoly();

            // Did a QD error occur whilst constructing a polygon?

            maGraphicsData.mnOSStatus = QDErr();

            if (    ( maGraphicsData.mnOSStatus == noErr )
                 && ( hPolygon != NULL )
               )
            {
                PaintPoly( hPolygon );

                KillPoly( hPolygon );
            } // if

            EndGraphics( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::DrawPolygon

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyPolygon( ULONG            nPoly,
                                   const ULONG     *pPoints,
                                   PCONSTSALPOINT  *ppPtAry
                                 )
{
    if  ( ( nPoly > 1 ) && ( ppPtAry != NULL ) && ( ppPtAry != NULL ) )
    {
        if ( BeginGraphics( &maGraphicsData ) )
        {
            RgnHandle   hPolyRgn   = NULL;
            RGBColor    aPolyColor = maGraphicsData.maBrushColor;
            OSStatus    nOSStatus  = noErr;

            RGBForeColor( &aPolyColor );

            // Construct a polygon region

            hPolyRgn = GetPolygonRgn(  nPoly,
                                       pPoints,
                                       ppPtAry,
                                      &nOSStatus
                                    );

            if ( ( hPolyRgn != NULL ) && ( nOSStatus == noErr ) )
            {
                MacPaintRgn( hPolyRgn );

                DisposeRgn( hPolyRgn );

                nOSStatus = QDErr();

                hPolyRgn = NULL;
            } // if

            maGraphicsData.mnOSStatus = nOSStatus;

            EndGraphics( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::DrawPolyPolygon

// -----------------------------------------------------------------------

void SalGraphics::CopyBits( const SalTwoRect  *pPosAry,
                            SalGraphics       *pSrcGraphics
                          )
{
    if ( ( pPosAry != NULL ) && ( pSrcGraphics != NULL ) )
    {
        if ( BeginGraphics( &maGraphicsData ) )
        {
            const BitMap  *pDstBitMap = GetPortBitMapForCopyBits( maGraphicsData.mpCGrafPort );

            if ( pDstBitMap != NULL )
            {
                Rect       aSrcRect;
                Rect       aDstRect;
                Rect       aPortBoundsRect;
                RgnHandle  hMaskRgn  = NULL;  // Mask Region for QD CopyBits
                short      nCopyMode = 0;

                SalTwoRect2QDSrcRect( pPosAry, &aSrcRect );

                SalTwoRect2QDDstRect( pPosAry, &aDstRect );

                GetPortBounds( pSrcGraphics->maGraphicsData.mpCGrafPort, &aPortBoundsRect );

                CheckRectBounds( &aPortBoundsRect, &aSrcRect, &aDstRect );

                nCopyMode = SelectCopyMode( &maGraphicsData );

                // Now we can call QD CopyBits to copy the bits from source rectangle
                // to the destination rectangle

                if ( LockGraphics( pSrcGraphics ) )
                {
                    const BitMap  *pSrcBitMap
                            = GetPortBitMapForCopyBits( pSrcGraphics->maGraphicsData.mpCGrafPort );

                    if ( pSrcBitMap != NULL )
                    {
                        ::CopyBits (  pSrcBitMap,
                                      pDstBitMap,
                                     &aSrcRect,
                                     &aDstRect,
                                      nCopyMode,
                                      hMaskRgn
                                   );
                    } // if

                    UnlockGraphics( pSrcGraphics );
                } // if
                else
                {
                    ::CopyBits (  pDstBitMap,
                                  pDstBitMap,
                                 &aSrcRect,
                                 &aDstRect,
                                  nCopyMode,
                                  hMaskRgn
                               );
                } // else
            } // if

            EndGraphics( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::CopyBits

// -----------------------------------------------------------------------

void SalGraphics::CopyArea( long    nDstX,
                            long    nDstY,
                            long    nSrcX,
                            long    nSrcY,
                            long    nSrcWidth,
                            long    nSrcHeight,
                            USHORT  nFlags
                          )
{
    #pragma unused(nFlags)

    if ( BeginGraphics( &maGraphicsData ) )
    {
        const BitMap  *pDstBitMap = GetPortBitMapForCopyBits( maGraphicsData.mpCGrafPort );

        if ( pDstBitMap != NULL )
        {
            short      nCopyMode  = 0;
            long       nDstWidth  = nSrcWidth;
            long       nDstHeight = nSrcHeight;
            RgnHandle  hMaskRgn   = NULL;  // Mask Region for QD CopyBits
            Rect       aSrcRect;
            Rect       aDstRect;
            Rect       aPortBoundsRect;

            GetQDRect( nSrcX, nSrcY, nSrcWidth, nSrcHeight, &aSrcRect );

            GetQDRect( nDstX, nDstY, nDstWidth, nDstHeight, &aDstRect );

            GetPortBounds( maGraphicsData.mpCGrafPort, &aPortBoundsRect );

            CheckRectBounds( &aPortBoundsRect, &aSrcRect, &aDstRect );

            nCopyMode = SelectCopyMode( &maGraphicsData );

            // Now we can call QD CopyBits to copy the bits from source rectangle
            // to the destination rectangle

            ::CopyBits (  pDstBitMap,
                          pDstBitMap,
                         &aSrcRect,
                         &aDstRect,
                          nCopyMode,
                          hMaskRgn
                       );
        } // if

        EndGraphics( &maGraphicsData );
    } // if
} // SalGraphics::CopyArea

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect*  pPosAry,
                              const SalBitmap&   rSalBitmap
                            )
{
    if ( pPosAry != NULL )
    {
        if ( BeginGraphics( &maGraphicsData ) )
        {
            SalGraphics *pSrcGraphics = rSalBitmap.GetGraphics();

            if ( LockGraphics( pSrcGraphics ) )
            {
                const BitMap  *pDstBitMap = GetPortBitMapForCopyBits( maGraphicsData.mpCGrafPort );

                if ( pDstBitMap != NULL )
                {
                    const BitMap  *pSrcBitMap
                            = GetPortBitMapForCopyBits( pSrcGraphics->maGraphicsData.mpCGrafPort );

                    if ( pSrcBitMap != NULL )
                    {
                        Rect       aSrcRect;
                        Rect       aDstRect;
                        Rect       aPortBoundsRect;
                        RgnHandle  hMaskRgn  = NULL;  // Mask Region for QD CopyBits
                        short      nCopyMode = 0;

                        SalTwoRect2QDSrcRect( pPosAry, &aSrcRect );

                        SalTwoRect2QDDstRect( pPosAry, &aDstRect );

                        GetPortBounds( pSrcGraphics->maGraphicsData.mpCGrafPort, &aPortBoundsRect );

                        CheckRectBounds( &aPortBoundsRect, &aSrcRect, &aDstRect );

                        nCopyMode = SelectCopyMode( &maGraphicsData );

                        // Now we can call QD CopyBits to copy the bits from source rectangle
                        // to the destination rectangle

                        ::CopyBits (  pSrcBitMap,
                                      pDstBitMap,
                                     &aSrcRect,
                                     &aDstRect,
                                      nCopyMode,
                                      hMaskRgn
                                   );
                    } // if
                } // if

                UnlockGraphics( pSrcGraphics );

                rSalBitmap.ReleaseGraphics( pSrcGraphics );
            } // if

            EndGraphics( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::DrawBitmap

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap&  rSalBitmap,
                              SalColor          nTransparentColor
                            )
{
    if ( pPosAry != NULL )
    {
        if ( BeginGraphics( &maGraphicsData ) )
        {
            SalGraphics *pSrcGraphics = rSalBitmap.GetGraphics();

            if ( LockGraphics( pSrcGraphics ) )
            {
                const BitMap  *pDstBitMap = GetPortBitMapForCopyBits( maGraphicsData.mpCGrafPort );

                if ( pDstBitMap != NULL )
                {
                    const BitMap  *pSrcBitMap
                            = GetPortBitMapForCopyBits( pSrcGraphics->maGraphicsData.mpCGrafPort );

                    if ( pSrcBitMap != NULL )
                    {
                        Rect       aSrcRect;
                        Rect       aDstRect;
                        Rect       aPortBoundsRect;
                        RGBColor   aBackColor;
                        RgnHandle  hMaskRgn  = NULL;  // Mask Region for QD CopyBits
                        short      nCopyMode = transparent;

                        SalTwoRect2QDSrcRect( pPosAry, &aSrcRect );

                        SalTwoRect2QDDstRect( pPosAry, &aDstRect );

                        GetPortBounds( pSrcGraphics->maGraphicsData.mpCGrafPort, &aPortBoundsRect );

                        CheckRectBounds( &aPortBoundsRect, &aSrcRect, &aDstRect );

                        aBackColor = SALColor2RGBColor( nTransparentColor );

                        RGBBackColor( &aBackColor );

                        // Now we can call QD CopyBits to copy the bits from
                        // source rectangle to the destination rectangle

                        ::CopyBits (  pSrcBitMap,
                                      pDstBitMap,
                                     &aSrcRect,
                                     &aDstRect,
                                      nCopyMode,
                                      hMaskRgn
                                   );
                    } // if
                } // if

                UnlockGraphics( pSrcGraphics );

                rSalBitmap.ReleaseGraphics( pSrcGraphics );
            } // if

            EndGraphics( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::DrawBitmap

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect*  pPosAry,
                              const SalBitmap&   rSalBitmap,
                              const SalBitmap&   rTransparentBitmap
                            )
{
    if ( pPosAry != NULL )
    {
        if ( BeginGraphics( &maGraphicsData ) )
        {
            SalGraphics *pSrcGraphics = rSalBitmap.GetGraphics();

            if ( LockGraphics( pSrcGraphics ) )
            {
                SalGraphics *pMskGraphics = rTransparentBitmap.GetGraphics();

                if (  LockGraphics( pMskGraphics ) )
                {
                    const BitMap  *pDstBitMap = GetPortBitMapForCopyBits( maGraphicsData.mpCGrafPort );

                    if ( pDstBitMap != NULL )
                    {
                        const BitMap  *pSrcBitMap
                                = GetPortBitMapForCopyBits( pSrcGraphics->maGraphicsData.mpCGrafPort );

                        if ( pSrcBitMap != NULL )
                        {
                            const BitMap  *pMskBitMap
                                    = GetPortBitMapForCopyBits( pSrcGraphics->maGraphicsData.mpCGrafPort );

                            if ( pMskBitMap != NULL )
                            {
                                Rect       aSrcRect;
                                Rect       aDstRect;
                                Rect       aPortBoundsRect;
                                RgnHandle  hMaskRgn  = NULL;  // Mask Region for QD CopyBits
                                short      nCopyMode = 0;

                                SalTwoRect2QDSrcRect( pPosAry, &aSrcRect );

                                SalTwoRect2QDDstRect( pPosAry, &aDstRect );

                                GetPortBounds(  pSrcGraphics->maGraphicsData.mpCGrafPort,
                                               &aPortBoundsRect
                                             );

                                CheckRectBounds( &aPortBoundsRect, &aSrcRect, &aDstRect );

                                nCopyMode = SelectCopyMode( &maGraphicsData );

                                // Now we can call QD CopyDeepMask to copy the bits from
                                // source rectangle to the destination rectangle using the
                                // the mask bits

                                ::CopyDeepMask(  pSrcBitMap,
                                                 pMskBitMap,
                                                 pDstBitMap,
                                                &aSrcRect,
                                                &aSrcRect,
                                                &aDstRect,
                                                 nCopyMode,
                                                 hMaskRgn
                                              );
                            } // if
                        } // if
                    } // if

                    UnlockGraphics( pMskGraphics );

                    rSalBitmap.ReleaseGraphics( pMskGraphics );
                } // if

                UnlockGraphics( pSrcGraphics );

                rSalBitmap.ReleaseGraphics( pSrcGraphics );
            } // if

            EndGraphics( &maGraphicsData );
        } // if
    } //if
} // SalGraphics::DrawBitmap

// -----------------------------------------------------------------------

void SalGraphics::DrawMask( const SalTwoRect*  pPosAry,
                            const SalBitmap&   rSalBitmap,
                            SalColor           nMaskColor
                          )
{
    if ( pPosAry != NULL )
    {
        if ( BeginGraphics( &maGraphicsData ) )
        {
            SalGraphics *pSrcGraphics = rSalBitmap.GetGraphics();

            if ( LockGraphics( pSrcGraphics ) )
            {
                const BitMap  *pDstBitMap
                        = GetPortBitMapForCopyBits( maGraphicsData.mpCGrafPort );

                if ( pDstBitMap != NULL )
                {
                    const BitMap  *pSrcBitMap
                            = GetPortBitMapForCopyBits( pSrcGraphics->maGraphicsData.mpCGrafPort );

                    if ( pSrcBitMap != NULL )
                    {
                        Rect       aSrcRect;
                        Rect       aDstRect;
                        Rect       aPortBoundsRect;
                        RGBColor   aMaskBackColor;
                        RgnHandle  hMaskRgn  = NULL;  // Mask Region for QD CopyBits
                        short      nCopyMode = 0;

                        SalTwoRect2QDSrcRect( pPosAry, &aSrcRect );

                        SalTwoRect2QDDstRect( pPosAry, &aDstRect );

                        GetPortBounds( pSrcGraphics->maGraphicsData.mpCGrafPort, &aPortBoundsRect );

                        CheckRectBounds( &aPortBoundsRect, &aSrcRect, &aDstRect );

                        aMaskBackColor = SALColor2RGBColor( nMaskColor );

                        RGBBackColor( &aMaskBackColor );

                        nCopyMode = SelectCopyMode( &maGraphicsData );

                        // Now we can call QD CopyDeepMask to copy the bits from
                        // source rectangle to the destination rectangle using the
                        // the mask bits

                        ::CopyDeepMask(  pSrcBitMap,
                                         pSrcBitMap,
                                         pDstBitMap,
                                        &aSrcRect,
                                        &aSrcRect,
                                        &aDstRect,
                                         nCopyMode,
                                         hMaskRgn
                                      );
                    } // if
                } // if

                UnlockGraphics( pSrcGraphics );

                rSalBitmap.ReleaseGraphics( pSrcGraphics );
            } // if

            EndGraphics( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::DrawMask

// -----------------------------------------------------------------------

SalBitmap* SalGraphics::GetBitmap( long  nX,
                                   long  nY,
                                   long  nDX,
                                   long  nDY
                                 )
{
    fprintf( stderr,
             "<<WARNING>> SalGraphics::GetBitmap not yet implemented!\n"
           );

    return NULL;
} // SalGraphics::GetBitmap

// -----------------------------------------------------------------------

SalColor SalGraphics::GetPixel( long nX,
                                long nY
                              )
{
    RGBColor  aRGBColor;
    SalColor  nSalColor       = 0;
    short     aHorizontalCoor = (short)nX;
    short     aVerticalCoor   = (short)nY;

    GetCPixel( aHorizontalCoor, aVerticalCoor, &aRGBColor );

    nSalColor = RGBColor2SALColor ( &aRGBColor );

    return nSalColor;
} // SalGraphics::GetPixel

// -----------------------------------------------------------------------

void SalGraphics::Invert( long       nX,
                          long       nY,
                          long       nWidth,
                          long       nHeight,
                          SalInvert  nFlags
                        )
{
    #pragma unused(nSalFlags)

    // Implementation not yet complete

    if ( BeginGraphics( &maGraphicsData ) )
    {
        short  left   = (short)nX;
        short  top    = (short)nY;
        short  right  = (short)nX + (short)nWidth;
        short  bottom = (short)nY + (short)nHeight;
        Rect   aRect;

        MacSetRect( &aRect, left, top, right, bottom );

        ::MacInvertRect( &aRect );

        EndGraphics( &maGraphicsData );
    } // if
} // SalGraphics::Invert

// -----------------------------------------------------------------------

void SalGraphics::Invert( ULONG            nPoints,
                          const SalPoint*  pPtAry,
                          SalInvert        nSalFlags
                        )
{
    #pragma unused(nSalFlags)

    // Implementation not yet complete

    if ( ( pPtAry != NULL ) && ( nPoints > 1 ) )
    {
        if ( BeginGraphics( &maGraphicsData ) )
        {
            unsigned long   nPolyEdges     = nPoints;
            unsigned long   nPolyEdgeIndex = 0;
            short           nX             = 0;
            short           nY             = 0;
            short           nPenMode       = patXor;
            RgnHandle       hPolyRgn       = NULL;
            CGrafPtr        pCGrafPort     = maGraphicsData.mpCGrafPort;
            Pattern         aPenPatGray;

            hPolyRgn = NewRgn ();

            if ( hPolyRgn != NULL )
            {
                GetQDGlobalsGray( &aPenPatGray );

                PenPat( &aPenPatGray );

                SetPortPenMode( pCGrafPort, nPenMode );

                // Begin region construction

                OpenRgn();

                    // Begin polygon regin construction

                    MoveTo( pPtAry[0].mnX,  pPtAry[0].mnY );

                    for ( nPolyEdgeIndex = 1;
                          nPolyEdgeIndex < nPolyEdges;
                          nPolyEdgeIndex++
                        )
                    {
                        MacLineTo( pPtAry[nPolyEdgeIndex].mnX,
                                   pPtAry[nPolyEdgeIndex].mnY
                                 );
                    } // for

                    MacLineTo( pPtAry[0].mnX,  pPtAry[0].mnY );

                    // End polygon region construction

                CloseRgn( hPolyRgn );

                // End region construction

                maGraphicsData.mnOSStatus = QDErr();

                if ( maGraphicsData.mnOSStatus == noErr )
                {
                    MacInvertRgn( hPolyRgn );

                    DisposeRgn( hPolyRgn );

                    hPolyRgn = NULL;
                } // if
            } // if

            EndGraphics( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::Invert

// -----------------------------------------------------------------------

BOOL SalGraphics::DrawEPS( long   nX,
                           long   nY,
                           long   nWidth,
                           long   nHeight,
                           void*  pPtr,
                           ULONG  nSize
                         )
{
    fprintf( stderr,
             "<<WARNING>> SalGraphics::DrawEPS not yet implemented!\n"
           );

    return FALSE;
} // SalGraphics::DrawEPS

// -----------------------------------------------------------------------

void SalGraphics::SetTextColor( SalColor nSalColor )
{
    maGraphicsData.maFontColor = SALColor2RGBColor( nSalColor );
} // SalGraphics::SetTextColor

// -----------------------------------------------------------------------

USHORT SalGraphics::SetFont( ImplFontSelectData* pFont )
{
    fprintf( stderr,
             "<<WARNING>> SalGraphics::SetFont not yet implemented!\n"
           );

    return 0;
} // SalGraphics::SetFont

// -----------------------------------------------------------------------

long SalGraphics::GetCharWidth( sal_Unicode  nChar1,
                                sal_Unicode  nChar2,
                                long*        pWidthAry
                              )
{
    // Stub code

    sal_Unicode nCharCount = nChar2 - nChar1 + 1;
    sal_Unicode i = 0;

    // Put a stub width in the pWidthAry that is passed to this method for
    // both of the two Unicode characters and all characters between them.
    // The width of nChar1 is put in element 0 of pWidthAry and the width
    // of nChar2 is put in element nChar2 - nChar1 of pWidthAry

    for ( i = 0 ; i < nCharCount ; i++ )
    {
        pWidthAry[i] = 10;
    } // for

    fprintf( stderr,
             "<<WARNING>> SalGraphics::GetCharWidth not yet implemented!\n"
           );

    return 1;
} // SalGraphics::GetCharWidth

// -----------------------------------------------------------------------

void SalGraphics::GetFontMetric( ImplFontMetricData* pMetric )
{
    // Stub Code

    pMetric->mnAscent  = 10;
    pMetric->mnDescent = 10;

    fprintf( stderr,
             "<<WARNING>> SalGraphics::GetFontMetric not yet implemented!\n"
           );
} // SalGraphics::GetFontMetric

// -----------------------------------------------------------------------

ULONG SalGraphics::GetKernPairs( ULONG              nPairs,
                                 ImplKernPairData*  pKernPairs
                               )
{
    fprintf( stderr,
             "<<WARNING>> SalGraphics::GetKernPairs not yet implemented!\n"
           );

    return 0;
} // SalGraphics::GetKernPairs

// -----------------------------------------------------------------------

void SalGraphics::GetDevFontList( ImplDevFontList* pList )
{
    ImplFontData *pFontData = new ImplFontData;

    pFontData->mnWidth  = 10;
    pFontData->mnHeight = 10;

    pList->Add( pFontData );

    fprintf( stderr,
             "<<WARNING>> SalGraphics::GetDevFontList not yet implemented!\n"
           );
} // SalGraphics::GetDevFontList

// -----------------------------------------------------------------------

void SalGraphics::DrawText( long                nX,
                            long                nY,
                            const xub_Unicode  *pStr,
                            xub_StrLen          nLen
                          )
{
    // The implementation is not yet complete

    if ( ( pStr != NULL ) && ( nLen > 0 ) )
    {
        if ( BeginGraphics( &maGraphicsData ) )
        {
            ByteString   aByteString( pStr,
                                      nLen,
                                      gsl_getSystemTextEncoding()
                                    );

            const char  *pTextBuffer = aByteString.GetBuffer();

            if ( pTextBuffer != NULL )
            {
                short           nFirstByte     = 0;
                short           nByteCount     = nLen;
                const RGBColor  aFontForeColor = maGraphicsData.maFontColor;

                RGBForeColor( &aFontForeColor );

                MoveTo( nX, nY );

                ::MacDrawText( pTextBuffer, nFirstByte, nByteCount );
            } // if

            EndGraphics( &maGraphicsData );
        } // if
    } // if
} // SalGraphics::DrawText

// -----------------------------------------------------------------------

void SalGraphics::DrawTextArray( long                nX,
                                 long                nY,
                                 const xub_Unicode*  pStr,
                                 xub_StrLen          nLen,
                                 const long*         pDXAry
                               )
{
    fprintf( stderr,
             "<<WARNING>> SalGraphics::DrawTextArray not yet implemented!\n"
           );
} // SalGraphics::DrawTextArray

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphBoundRect( xub_Unicode  cChar,
                                     long*        pX,
                                     long*        pY,
                                     long*        pWidth,
                                     long*        pHeight
                                   )
{
    fprintf( stderr,
             "<<WARNING>> SalGraphics::GetGlyphBoundRect not yet implemented!\n"
           );

    return FALSE;
} // SalGraphics::GetGlyphBoundRect

// -----------------------------------------------------------------------

ULONG SalGraphics::GetGlyphOutline( xub_Unicode  cChar,
                                    USHORT**     ppPolySizes,
                                    SalPoint**   ppPoints,
                                    BYTE**       ppFlags
                                  )
{
    fprintf( stderr,
             "<<WARNING>> SalGraphics::GetGlyphOutline not yet implemented!\n"
           );

    return 0;
} // SalGraphics::GetGlyphOutline

// =======================================================================

// =======================================================================
