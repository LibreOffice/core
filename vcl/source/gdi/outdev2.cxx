/*************************************************************************
 *
 *  $RCSfile: outdev2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-10-26 08:46:33 $
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

#define _SV_OUTDEV2_CXX

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_IMPBMP_HXX
#include <impbmp.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <bitmapex.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_OUTDATA_HXX
#include <outdata.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#ifndef _SV_REGION_H
#include <region.h>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifdef REMOTE_APPSERVER
#include <rmoutdev.hxx>
#endif

#define BAND_MAX_SIZE 512000

// =======================================================================

DBG_NAMEEX( OutputDevice );

// =======================================================================

// -----------
// - Defines -
// -----------

#ifndef REMOTE_APPSERVER

#define OUTDEV_INIT()                       \
{                                           \
    if ( !IsDeviceOutputNecessary() )       \
        return;                             \
                                            \
    if ( !mpGraphics )                      \
        if ( !ImplGetGraphics() )           \
            return;                         \
                                            \
    if ( mbInitClipRegion )                 \
        ImplInitClipRegion();               \
                                            \
    if ( mbOutputClipped )                  \
        return;                             \
}

#else // !REMOTE_APPSERVER

#define OUTDEV_INIT()                                           \
{                                                               \
    if ( !IsDeviceOutputNecessary() )                           \
        return;                                                 \
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();    \
    if ( !pGraphics )                                           \
        return;                                                 \
}

#endif // REMOTE_APPSERVER

#ifndef REMOTE_APPSERVER
#define TwoRect     SalTwoRect
#else
#define TwoRect     RemoteTwoRect
#endif

// -------------
// - externals -
// -------------

extern ULONG nVCLRLut[ 6 ];
extern ULONG nVCLGLut[ 6 ];
extern ULONG nVCLBLut[ 6 ];
extern ULONG nVCLDitherLut[ 256 ];
extern ULONG nVCLLut[ 256 ];

// =======================================================================

ULONG ImplAdjustTwoRect( TwoRect& rTwoRect, const Size& rSizePix )
{
    ULONG nMirrFlags = 0;

    if ( rTwoRect.mnDestWidth < 0 )
    {
        rTwoRect.mnSrcX = rSizePix.Width() - rTwoRect.mnSrcX - rTwoRect.mnSrcWidth;
        rTwoRect.mnDestWidth = -rTwoRect.mnDestWidth;
        rTwoRect.mnDestX -= rTwoRect.mnDestWidth-1;
        nMirrFlags |= BMP_MIRROR_HORZ;
    }

    if ( rTwoRect.mnDestHeight < 0 )
    {
        rTwoRect.mnSrcY = rSizePix.Height() - rTwoRect.mnSrcY - rTwoRect.mnSrcHeight;
        rTwoRect.mnDestHeight = -rTwoRect.mnDestHeight;
        rTwoRect.mnDestY -= rTwoRect.mnDestHeight-1;
        nMirrFlags |= BMP_MIRROR_VERT;
    }

    if( ( rTwoRect.mnSrcX < 0 ) || ( rTwoRect.mnSrcX >= rSizePix.Width() ) ||
        ( rTwoRect.mnSrcY < 0 ) || ( rTwoRect.mnSrcY >= rSizePix.Height() ) ||
        ( ( rTwoRect.mnSrcX + rTwoRect.mnSrcWidth ) > rSizePix.Width() ) ||
        ( ( rTwoRect.mnSrcY + rTwoRect.mnSrcHeight ) > rSizePix.Height() ) )
    {
        const Rectangle aSourceRect( Point( rTwoRect.mnSrcX, rTwoRect.mnSrcY ),
                                     Size( rTwoRect.mnSrcWidth, rTwoRect.mnSrcHeight ) );
        Rectangle       aCropRect( aSourceRect );

        aCropRect.Intersection( Rectangle( Point(), rSizePix ) );

        if( aCropRect.IsEmpty() )
            rTwoRect.mnSrcWidth = rTwoRect.mnSrcHeight = rTwoRect.mnDestWidth = rTwoRect.mnDestHeight = 0;
        else
        {
            const double    fFactorX = ( rTwoRect.mnSrcWidth > 1 ) ? (double) ( rTwoRect.mnDestWidth - 1 ) / ( rTwoRect.mnSrcWidth - 1 ) : 0.0;
            const double    fFactorY = ( rTwoRect.mnSrcHeight > 1 ) ? (double) ( rTwoRect.mnDestHeight - 1 ) / ( rTwoRect.mnSrcHeight - 1 ) : 0.0;

            const long nDstX1 = rTwoRect.mnDestX + FRound( fFactorX * ( aCropRect.Left() - rTwoRect.mnSrcX ) );
            const long nDstY1 = rTwoRect.mnDestY + FRound( fFactorY * ( aCropRect.Top() - rTwoRect.mnSrcY ) );
            const long nDstX2 = rTwoRect.mnDestX + FRound( fFactorX * ( aCropRect.Right() - rTwoRect.mnSrcX ) );
            const long nDstY2 = rTwoRect.mnDestY + FRound( fFactorY * ( aCropRect.Bottom() - rTwoRect.mnSrcY ) );

            rTwoRect.mnSrcX = aCropRect.Left();
            rTwoRect.mnSrcY = aCropRect.Top();
            rTwoRect.mnSrcWidth = aCropRect.GetWidth();
            rTwoRect.mnSrcHeight = aCropRect.GetHeight();
            rTwoRect.mnDestX = nDstX1;
            rTwoRect.mnDestY = nDstY1;
            rTwoRect.mnDestWidth = nDstX2 - nDstX1 + 1;
            rTwoRect.mnDestHeight = nDstY2 - nDstY1 + 1;
        }
    }

    return nMirrFlags;
}

// =======================================================================

void OutputDevice::ImplDrawOutDevDirect( const OutputDevice* pSrcDev, void* pVoidPosAry )
{
    TwoRect*            pPosAry = (TwoRect*)pVoidPosAry;
#ifndef REMOTE_APPSERVER
    SalGraphics*        pGraphics2;
#else
    ImplServerGraphics* pGraphics2;
#endif

    if ( pPosAry->mnSrcWidth && pPosAry->mnSrcHeight && pPosAry->mnDestWidth && pPosAry->mnDestHeight )
    {
        if ( this == pSrcDev )
            pGraphics2 = NULL;
        else
        {
            if ( (GetOutDevType() != pSrcDev->GetOutDevType()) ||
                 (GetOutDevType() != OUTDEV_WINDOW) )
            {
#ifndef REMOTE_APPSERVER
                if ( !pSrcDev->mpGraphics )
                {
                    if ( !((OutputDevice*)pSrcDev)->ImplGetGraphics() )
                        return;
                }
#endif
                pGraphics2 = pSrcDev->mpGraphics;
            }
            else
            {
                if ( ((Window*)this)->mpFrameWindow == ((Window*)pSrcDev)->mpFrameWindow )
                    pGraphics2 = NULL;
                else
                {
#ifndef REMOTE_APPSERVER
                    if ( !pSrcDev->mpGraphics )
                    {
                        if ( !((OutputDevice*)pSrcDev)->ImplGetGraphics() )
                            return;
                    }
#endif
                    pGraphics2 = pSrcDev->mpGraphics;

#ifndef REMOTE_APPSERVER
                    if ( !mpGraphics )
                    {
                        if ( !ImplGetGraphics() )
                            return;
                    }
                    DBG_ASSERT( mpGraphics && pSrcDev->mpGraphics,
                                "OutputDevice::DrawOutDev(): We need more than one Graphics" );
#endif
                }
            }
        }

        Rectangle   aSrcOutRect( Point( pSrcDev->mnOutOffX, pSrcDev->mnOutOffY ),
                                 Size( pSrcDev->mnOutWidth, pSrcDev->mnOutHeight ) );
        Rectangle   aSrcRect( Point( pPosAry->mnSrcX, pPosAry->mnSrcY ),
                              Size( pPosAry->mnSrcWidth, pPosAry->mnSrcHeight ) );
        const long  nOldRight = aSrcRect.Right();
        const long  nOldBottom = aSrcRect.Bottom();

        if ( !aSrcRect.Intersection( aSrcOutRect ).IsEmpty() )
        {
            if ( (pPosAry->mnSrcX+pPosAry->mnSrcWidth-1) > aSrcOutRect.Right() )
            {
                const long nOldWidth = pPosAry->mnSrcWidth;
                pPosAry->mnSrcWidth -= (nOldRight - aSrcRect.Right());
                pPosAry->mnDestWidth = pPosAry->mnDestWidth * pPosAry->mnSrcWidth / nOldWidth;
            }

            if ( (pPosAry->mnSrcY+pPosAry->mnSrcHeight-1) > aSrcOutRect.Bottom() )
            {
                const long nOldHeight = pPosAry->mnSrcHeight;
                pPosAry->mnSrcHeight -= (nOldBottom - aSrcRect.Bottom());
                pPosAry->mnDestHeight = pPosAry->mnDestHeight * pPosAry->mnSrcHeight / nOldHeight;
            }

            mpGraphics->CopyBits( pPosAry, pGraphics2 );
        }
    }
}

// ------------------------------------------------------------------

void OutputDevice::DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                               const Point& rSrcPt,  const Size& rSrcSize )
{
    DBG_TRACE( "OutputDevice::DrawOutDev()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_ASSERT( meOutDevType != OUTDEV_PRINTER, "Don't use OutputDevice::DrawOutDev(...) with printer devices!" );

    if ( meOutDevType == OUTDEV_PRINTER )
        return;

    if ( ROP_INVERT == meRasterOp )
    {
        DrawRect( Rectangle( rDestPt, rDestSize ) );
        return;
    }

    if ( mpMetaFile )
    {
        const Bitmap aBmp( GetBitmap( rSrcPt, rSrcSize ) );
        mpMetaFile->AddAction( new MetaBmpScaleAction( rDestPt, rDestSize, aBmp ) );
    }

    OUTDEV_INIT();

    TwoRect aPosAry;
    aPosAry.mnSrcWidth   = ImplLogicWidthToDevicePixel( rSrcSize.Width() );
    aPosAry.mnSrcHeight  = ImplLogicHeightToDevicePixel( rSrcSize.Height() );
    aPosAry.mnDestWidth  = ImplLogicWidthToDevicePixel( rDestSize.Width() );
    aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

    if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
    {
        aPosAry.mnSrcX       = ImplLogicXToDevicePixel( rSrcPt.X() );
        aPosAry.mnSrcY       = ImplLogicYToDevicePixel( rSrcPt.Y() );
        aPosAry.mnDestX      = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY      = ImplLogicYToDevicePixel( rDestPt.Y() );

        Rectangle   aSrcOutRect( Point( mnOutOffX, mnOutOffY ),
                                 Size( mnOutWidth, mnOutHeight ) );
        Rectangle   aSrcRect( Point( aPosAry.mnSrcX, aPosAry.mnSrcY ),
                              Size( aPosAry.mnSrcWidth, aPosAry.mnSrcHeight ) );
        long        nOldRight = aSrcRect.Right();
        long        nOldBottom = aSrcRect.Bottom();

        if ( !aSrcRect.Intersection( aSrcOutRect ).IsEmpty() )
        {
            if ( (aPosAry.mnSrcX+aPosAry.mnSrcWidth-1) > aSrcOutRect.Right() )
            {
                long nOldWidth = aPosAry.mnSrcWidth;
                aPosAry.mnSrcWidth -= nOldRight-aSrcRect.Right();
                aPosAry.mnDestWidth = aPosAry.mnDestWidth*aPosAry.mnSrcWidth/nOldWidth;
            }

            if ( (aPosAry.mnSrcY+aPosAry.mnSrcHeight-1) > aSrcOutRect.Bottom() )
            {
                long nOldHeight = aPosAry.mnSrcHeight;
                aPosAry.mnSrcHeight -= nOldBottom-aSrcRect.Bottom();
                aPosAry.mnDestHeight = aPosAry.mnDestHeight*aPosAry.mnSrcHeight/nOldHeight;
            }

            mpGraphics->CopyBits( &aPosAry, NULL );
        }
    }
}

// ------------------------------------------------------------------

void OutputDevice::DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                               const Point& rSrcPt,  const Size& rSrcSize,
                               const OutputDevice& rOutDev )
{
    DBG_TRACE( "OutputDevice::DrawOutDev()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rOutDev, OutputDevice, ImplDbgCheckOutputDevice );
    DBG_ASSERT( meOutDevType != OUTDEV_PRINTER, "Don't use OutputDevice::DrawOutDev(...) with printer devices!" );
    DBG_ASSERT( rOutDev.meOutDevType != OUTDEV_PRINTER, "Don't use OutputDevice::DrawOutDev(...) with printer devices!" );

    if ( (meOutDevType == OUTDEV_PRINTER) || (rOutDev.meOutDevType == OUTDEV_PRINTER) )
        return;

    if ( ROP_INVERT == meRasterOp )
    {
        DrawRect( Rectangle( rDestPt, rDestSize ) );
        return;
    }

    if ( mpMetaFile )
    {
        const Bitmap aBmp( rOutDev.GetBitmap( rSrcPt, rSrcSize ) );
        mpMetaFile->AddAction( new MetaBmpScaleAction( rDestPt, rDestSize, aBmp ) );
    }

    OUTDEV_INIT();

    TwoRect aPosAry;
    aPosAry.mnSrcX       = rOutDev.ImplLogicXToDevicePixel( rSrcPt.X() );
    aPosAry.mnSrcY       = rOutDev.ImplLogicYToDevicePixel( rSrcPt.Y() );
    aPosAry.mnSrcWidth   = rOutDev.ImplLogicWidthToDevicePixel( rSrcSize.Width() );
    aPosAry.mnSrcHeight  = rOutDev.ImplLogicHeightToDevicePixel( rSrcSize.Height() );
    aPosAry.mnDestX      = ImplLogicXToDevicePixel( rDestPt.X() );
    aPosAry.mnDestY      = ImplLogicYToDevicePixel( rDestPt.Y() );
    aPosAry.mnDestWidth  = ImplLogicWidthToDevicePixel( rDestSize.Width() );
    aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

    ImplDrawOutDevDirect( &rOutDev, &aPosAry );
}

// ------------------------------------------------------------------

void OutputDevice::CopyArea( const Point& rDestPt,
                             const Point& rSrcPt,  const Size& rSrcSize,
                             USHORT nFlags )
{
    DBG_TRACE( "OutputDevice::CopyArea()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_ASSERT( meOutDevType != OUTDEV_PRINTER, "Don't use OutputDevice::CopyArea(...) with printer devices!" );

    if ( meOutDevType == OUTDEV_PRINTER )
        return;

    RasterOp eOldRop = GetRasterOp();
    SetRasterOp( ROP_OVERPAINT );

    OUTDEV_INIT();

    TwoRect aPosAry;
    aPosAry.mnSrcWidth   = ImplLogicWidthToDevicePixel( rSrcSize.Width() );
    aPosAry.mnSrcHeight  = ImplLogicHeightToDevicePixel( rSrcSize.Height() );

    if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight )
    {
        aPosAry.mnSrcX       = ImplLogicXToDevicePixel( rSrcPt.X() );
        aPosAry.mnSrcY       = ImplLogicYToDevicePixel( rSrcPt.Y() );
        aPosAry.mnDestX      = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY      = ImplLogicYToDevicePixel( rDestPt.Y() );

        Rectangle   aSrcOutRect( Point( mnOutOffX, mnOutOffY ),
                                 Size( mnOutWidth, mnOutHeight ) );
        Rectangle   aSrcRect( Point( aPosAry.mnSrcX, aPosAry.mnSrcY ),
                              Size( aPosAry.mnSrcWidth, aPosAry.mnSrcHeight ) );
        long        nOldRight = aSrcRect.Right();
        long        nOldBottom = aSrcRect.Bottom();

        if ( !aSrcRect.Intersection( aSrcOutRect ).IsEmpty() )
        {
            if ( (aPosAry.mnSrcX+aPosAry.mnSrcWidth-1) > aSrcOutRect.Right() )
                aPosAry.mnSrcWidth -= nOldRight-aSrcRect.Right();

            if ( (aPosAry.mnSrcY+aPosAry.mnSrcHeight-1) > aSrcOutRect.Bottom() )
                aPosAry.mnSrcHeight -= nOldBottom-aSrcRect.Bottom();

            if ( (meOutDevType == OUTDEV_WINDOW) && (nFlags & COPYAREA_WINDOWINVALIDATE) )
            {
                ((Window*)this)->ImplMoveAllInvalidateRegions( aSrcRect,
                                                               aPosAry.mnDestX-aPosAry.mnSrcX,
                                                               aPosAry.mnDestY-aPosAry.mnSrcY,
                                                               FALSE );

#ifndef REMOTE_APPSERVER
                mpGraphics->CopyArea( aPosAry.mnDestX, aPosAry.mnDestY,
                                      aPosAry.mnSrcX, aPosAry.mnSrcY,
                                      aPosAry.mnSrcWidth, aPosAry.mnSrcHeight,
                                      SAL_COPYAREA_WINDOWINVALIDATE );
#else
                mpGraphics->CopyArea( aPosAry.mnDestX, aPosAry.mnDestY,
                                      aPosAry.mnSrcX, aPosAry.mnSrcY,
                                      aPosAry.mnSrcWidth, aPosAry.mnSrcHeight,
                                      COPYAREA_WINDOWINVALIDATE );
#endif
            }
            else
            {
                aPosAry.mnDestWidth  = aPosAry.mnSrcWidth;
                aPosAry.mnDestHeight = aPosAry.mnSrcHeight;
                mpGraphics->CopyBits( &aPosAry, NULL );
            }
        }
    }

    SetRasterOp( eOldRop );
}

// ------------------------------------------------------------------

void OutputDevice::ImplDrawFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                     const OutputDevice& rOutDev, const Region& rRegion )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    GDIMetaFile*    pOldMetaFile = mpMetaFile;
    BOOL            bOldMap = mbMap;
    RasterOp        eOldROP = GetRasterOp();
    mpMetaFile = NULL;
    mbMap = FALSE;
    SetRasterOp( ROP_OVERPAINT );

#ifndef REMOTE_APPSERVER
    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }
#else
    if ( !IsDeviceOutputNecessary() )
        return;

#endif

    // ClipRegion zuruecksetzen
#ifndef REMOTE_APPSERVER
    if ( rRegion.IsNull() )
        mpGraphics->ResetClipRegion();
    else
        ImplSelectClipRegion( mpGraphics, rRegion );
#else
    if ( rRegion.IsNull() )
        mpGraphics->SetClipRegion();
    else
        mpGraphics->SetClipRegion( rRegion );
#endif

    TwoRect aPosAry;
    aPosAry.mnSrcX       = rDevPt.X();
    aPosAry.mnSrcY       = rDevPt.Y();
    aPosAry.mnSrcWidth   = rDevSize.Width();
    aPosAry.mnSrcHeight  = rDevSize.Height();
    aPosAry.mnDestX      = rPt.X();
    aPosAry.mnDestY      = rPt.Y();
    aPosAry.mnDestWidth  = rDevSize.Width();
    aPosAry.mnDestHeight = rDevSize.Height();
    ImplDrawOutDevDirect( &rOutDev, &aPosAry );

    // Dafuer sorgen, das ClipRegion neu berechnet und gesetzt wird
    mbInitClipRegion = TRUE;

    SetRasterOp( eOldROP );
    mbMap = bOldMap;
    mpMetaFile = pOldMetaFile;
}

// ------------------------------------------------------------------

void OutputDevice::ImplGetFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                    OutputDevice& rDev )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    BOOL bOldMap = mbMap;
    mbMap = FALSE;
    rDev.DrawOutDev( rDevPt, rDevSize, rPt, rDevSize, *this );
    mbMap = bOldMap;
}

// ------------------------------------------------------------------

void OutputDevice::DrawBitmap( const Point& rDestPt, const Bitmap& rBitmap )
{
    DBG_TRACE( "OutputDevice::DrawBitmap()" );
    const Size aSizePix( rBitmap.GetSizePixel() );
    ImplDrawBitmap( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmap, META_BMP_ACTION );
}

// ------------------------------------------------------------------

void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap )
{
    DBG_TRACE( "OutputDevice::DrawBitmap( Size )" );
    ImplDrawBitmap( rDestPt, rDestSize, Point(), rBitmap.GetSizePixel(), rBitmap, META_BMPSCALE_ACTION );
}

// ------------------------------------------------------------------

void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize,
                               const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                               const Bitmap& rBitmap )
{
    DBG_TRACE( "OutputDevice::DrawBitmap( Point, Size )" );
    ImplDrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmap, META_BMPSCALEPART_ACTION );
}

// -----------------------------------------------------------------------------

void OutputDevice::ImplDrawBitmap( const Point& rDestPt, const Size& rDestSize,
                                   const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                   const Bitmap& rBitmap, const ULONG nAction )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Bitmap aBmp( rBitmap );

    if ( ( mnDrawMode & DRAWMODE_NOBITMAP ) )
        return;
    else if ( ROP_INVERT == meRasterOp )
    {
        DrawRect( Rectangle( rDestPt, rDestSize ) );
        return;
    }
    else if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP |
                             DRAWMODE_GRAYBITMAP | DRAWMODE_GHOSTEDBITMAP ) )
    {
        if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP ) )
        {
            BYTE cCmpVal;

            if ( mnDrawMode & DRAWMODE_BLACKBITMAP )
                cCmpVal = ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP ) ? 0x80 : 0;
            else
                cCmpVal = 255;

            Color aCol( cCmpVal, cCmpVal, cCmpVal );
            Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            SetLineColor( aCol );
            SetFillColor( aCol );
            DrawRect( Rectangle( rDestPt, rDestSize ) );
            Pop();
            return;
        }
        else if( !!aBmp )
        {
            if ( mnDrawMode & DRAWMODE_GRAYBITMAP )
                aBmp.Convert( BMP_CONVERSION_8BIT_GREYS );

            if ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP )
                aBmp.Convert( BMP_CONVERSION_GHOSTED );
        }
    }

    if ( mpMetaFile )
    {
        switch( nAction )
        {
            case( META_BMP_ACTION ):
                mpMetaFile->AddAction( new MetaBmpAction( rDestPt, aBmp ) );
            break;

            case( META_BMPSCALE_ACTION ):
                mpMetaFile->AddAction( new MetaBmpScaleAction( rDestPt, rDestSize, aBmp ) );
            break;

            case( META_BMPSCALEPART_ACTION ):
                mpMetaFile->AddAction( new MetaBmpScalePartAction(
                    rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, aBmp ) );
            break;
        }
    }

    OUTDEV_INIT();

    if( ( OUTDEV_PRINTER == meOutDevType ) && mbClipRegion && ( REGION_COMPLEX == maRegion.GetType() ) )
    {
        Bitmap aMask;
        ImplPrintTransparent( aBmp, aMask, rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );
        return;
    }

    if ( !( !aBmp ) )
    {
        TwoRect aPosAry;

        aPosAry.mnSrcX = rSrcPtPixel.X();
        aPosAry.mnSrcY = rSrcPtPixel.Y();
        aPosAry.mnSrcWidth = rSrcSizePixel.Width();
        aPosAry.mnSrcHeight = rSrcSizePixel.Height();
        aPosAry.mnDestX = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY = ImplLogicYToDevicePixel( rDestPt.Y() );
        aPosAry.mnDestWidth = ImplLogicWidthToDevicePixel( rDestSize.Width() );
        aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

        const ULONG nMirrFlags = ImplAdjustTwoRect( aPosAry, aBmp.GetSizePixel() );

        if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
        {
            if ( nMirrFlags )
                aBmp.Mirror( nMirrFlags );

#ifndef REMOTE_APPSERVER
            mpGraphics->DrawBitmap( &aPosAry, *aBmp.ImplGetImpBitmap()->ImplGetSalBitmap() );
#else
            aBmp.ImplDrawRemote( this,
                        Point( aPosAry.mnSrcX, aPosAry.mnSrcY ),
                        Size( aPosAry.mnSrcWidth, aPosAry.mnSrcHeight ),
                        Point( aPosAry.mnDestX, aPosAry.mnDestY ),
                        Size( aPosAry.mnDestWidth, aPosAry.mnDestHeight ) );
#endif
        }
    }
}

// ------------------------------------------------------------------

void OutputDevice::DrawBitmapEx( const Point& rDestPt,
                                 const BitmapEx& rBitmapEx )
{
    DBG_TRACE( "OutputDevice::DrawBitmapEx()" );

    if( TRANSPARENT_NONE == rBitmapEx.GetTransparentType() )
        DrawBitmap( rDestPt, rBitmapEx.GetBitmap() );
    else
    {
        const Size aSizePix( rBitmapEx.GetSizePixel() );
        ImplDrawBitmapEx( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmapEx, META_BMPEX_ACTION );
    }
}

// ------------------------------------------------------------------

void OutputDevice::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                 const BitmapEx& rBitmapEx )
{
    DBG_TRACE( "OutputDevice::DrawBitmapEx( Size )" );

    if ( TRANSPARENT_NONE == rBitmapEx.GetTransparentType() )
        DrawBitmap( rDestPt, rDestSize, rBitmapEx.GetBitmap() );
    else
        ImplDrawBitmapEx( rDestPt, rDestSize, Point(), rBitmapEx.GetSizePixel(), rBitmapEx, META_BMPEXSCALE_ACTION );
}

// ------------------------------------------------------------------

void OutputDevice::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                 const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                 const BitmapEx& rBitmapEx )
{
    DBG_TRACE( "OutputDevice::DrawBitmapEx( Point, Size )" );

    if( TRANSPARENT_NONE == rBitmapEx.GetTransparentType() )
        DrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmapEx.GetBitmap() );
    else
        ImplDrawBitmapEx( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmapEx, META_BMPEXSCALEPART_ACTION );
}

// ------------------------------------------------------------------

void OutputDevice::ImplDrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                     const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                     const BitmapEx& rBitmapEx, const ULONG nAction )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    BitmapEx aBmpEx( rBitmapEx );

    if ( mnDrawMode & DRAWMODE_NOBITMAP )
        return;
    else if ( ROP_INVERT == meRasterOp )
    {
        DrawRect( Rectangle( rDestPt, rDestSize ) );
        return;
    }
    else if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP |
                             DRAWMODE_GRAYBITMAP | DRAWMODE_GHOSTEDBITMAP ) )
    {
        if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP ) )
        {
            Bitmap  aColorBmp( aBmpEx.GetSizePixel(), ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP ) ? 4 : 1 );
            BYTE    cCmpVal;

            if ( mnDrawMode & DRAWMODE_BLACKBITMAP )
                cCmpVal = ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP ) ? 0x80 : 0;
            else
                cCmpVal = 255;

            aColorBmp.Erase( Color( cCmpVal, cCmpVal, cCmpVal ) );

            if( aBmpEx.IsAlpha() )
                aBmpEx = BitmapEx( aColorBmp, aBmpEx.GetAlpha() );
            else
                aBmpEx = BitmapEx( aColorBmp, aBmpEx.GetMask() );
        }
        else if( !!aBmpEx )
        {
            if ( mnDrawMode & DRAWMODE_GRAYBITMAP )
                aBmpEx.Convert( BMP_CONVERSION_8BIT_GREYS );

            if ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP )
                aBmpEx.Convert( BMP_CONVERSION_GHOSTED );
        }
    }

    if ( mpMetaFile )
    {
        switch( nAction )
        {
            case( META_BMPEX_ACTION ):
                mpMetaFile->AddAction( new MetaBmpExAction( rDestPt, aBmpEx ) );
            break;

            case( META_BMPEXSCALE_ACTION ):
                mpMetaFile->AddAction( new MetaBmpExScaleAction( rDestPt, rDestSize, aBmpEx ) );
            break;

            case( META_BMPEXSCALEPART_ACTION ):
                mpMetaFile->AddAction( new MetaBmpExScalePartAction( rDestPt, rDestSize,
                                                                     rSrcPtPixel, rSrcSizePixel, aBmpEx ) );
            break;
        }
    }

    OUTDEV_INIT();

    if( OUTDEV_PRINTER == meOutDevType )
    {
        Bitmap aBmp( aBmpEx.GetBitmap() ), aMask( aBmpEx.GetMask() );
        aBmp.Replace( aMask, Color( COL_WHITE ) );
        ImplPrintTransparent( aBmp, aMask, rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );
        return;
    }
#ifndef REMOTE_APPSERVER
    else if( rBitmapEx.IsAlpha() )
    {
        ImplDrawAlpha( aBmpEx.GetBitmap(), aBmpEx.GetAlpha(), rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );
        return;
    }
#endif

    if( !( !aBmpEx ) )
    {
        TwoRect aPosAry;

        aPosAry.mnSrcX = rSrcPtPixel.X();
        aPosAry.mnSrcY = rSrcPtPixel.Y();
        aPosAry.mnSrcWidth = rSrcSizePixel.Width();
        aPosAry.mnSrcHeight = rSrcSizePixel.Height();
        aPosAry.mnDestX = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY = ImplLogicYToDevicePixel( rDestPt.Y() );
        aPosAry.mnDestWidth = ImplLogicWidthToDevicePixel( rDestSize.Width() );
        aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

        const ULONG nMirrFlags = ImplAdjustTwoRect( aPosAry, aBmpEx.GetSizePixel() );

        if( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
        {
#ifndef REMOTE_APPSERVER

            if( nMirrFlags )
                aBmpEx.Mirror( nMirrFlags );

            const ImpBitmap* pImpBmp = aBmpEx.ImplGetBitmapImpBitmap();
            const ImpBitmap* pMaskBmp = aBmpEx.ImplGetMaskImpBitmap();

            if ( pMaskBmp )
                mpGraphics->DrawBitmap( &aPosAry, *pImpBmp->ImplGetSalBitmap(), *pMaskBmp->ImplGetSalBitmap() );
            else
                mpGraphics->DrawBitmap( &aPosAry, *pImpBmp->ImplGetSalBitmap() );

#else

            if( nMirrFlags )
                aBmpEx.Mirror( nMirrFlags );

            aBmpEx.ImplDrawRemote( this,
                    Point( aPosAry.mnSrcX, aPosAry.mnSrcY ),
                    Size( aPosAry.mnSrcWidth, aPosAry.mnSrcHeight ),
                    Point( aPosAry.mnDestX, aPosAry.mnDestY ),
                    Size( aPosAry.mnDestWidth, aPosAry.mnDestHeight ) );

#endif
        }
    }
}

// ------------------------------------------------------------------

void OutputDevice::DrawMask( const Point& rDestPt,
                             const Bitmap& rBitmap, const Color& rMaskColor )
{
    DBG_TRACE( "OutputDevice::DrawMask()" );
    const Size aSizePix( rBitmap.GetSizePixel() );
    ImplDrawMask( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmap, rMaskColor, META_MASK_ACTION );
}

// ------------------------------------------------------------------

void OutputDevice::DrawMask( const Point& rDestPt, const Size& rDestSize,
                             const Bitmap& rBitmap, const Color& rMaskColor )
{
    DBG_TRACE( "OutputDevice::DrawMask( Size )" );
    ImplDrawMask( rDestPt, rDestSize, Point(), rBitmap.GetSizePixel(), rBitmap, rMaskColor, META_MASKSCALE_ACTION );
}

// ------------------------------------------------------------------

void OutputDevice::DrawMask( const Point& rDestPt, const Size& rDestSize,
                             const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                             const Bitmap& rBitmap, const Color& rMaskColor )
{
    DBG_TRACE( "OutputDevice::DrawMask( Point, Size )" );
    ImplDrawMask( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmap, rMaskColor, META_MASKSCALEPART_ACTION );
}

// ------------------------------------------------------------------

void OutputDevice::ImplDrawMask( const Point& rDestPt, const Size& rDestSize,
                                 const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                 const Bitmap& rBitmap, const Color& rMaskColor,
                                 const ULONG nAction )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if( ROP_INVERT == meRasterOp )
    {
        DrawRect( Rectangle( rDestPt, rDestSize ) );
        return;
    }

    if ( mpMetaFile )
    {
        switch( nAction )
        {
            case( META_MASK_ACTION ):
                mpMetaFile->AddAction( new MetaMaskAction( rDestPt,
                    rBitmap, rMaskColor ) );
            break;

            case( META_MASKSCALE_ACTION ):
                mpMetaFile->AddAction( new MetaMaskScaleAction( rDestPt,
                    rDestSize, rBitmap, rMaskColor ) );
            break;

            case( META_MASKSCALEPART_ACTION ):
                mpMetaFile->AddAction( new MetaMaskScalePartAction( rDestPt, rDestSize,
                    rSrcPtPixel, rSrcSizePixel, rBitmap, rMaskColor ) );
            break;
        }
    }

    OUTDEV_INIT();

#ifndef REMOTE_APPSERVER
    if ( OUTDEV_PRINTER == meOutDevType )
    {
        ImplPrintMask( rBitmap, rMaskColor, rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );
        return;
    }
#endif

    const ImpBitmap* pImpBmp = rBitmap.ImplGetImpBitmap();

    if ( pImpBmp )
    {
        TwoRect aPosAry;

        aPosAry.mnSrcX = rSrcPtPixel.X();
        aPosAry.mnSrcY = rSrcPtPixel.Y();
        aPosAry.mnSrcWidth = rSrcSizePixel.Width();
        aPosAry.mnSrcHeight = rSrcSizePixel.Height();
        aPosAry.mnDestX = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY = ImplLogicYToDevicePixel( rDestPt.Y() );
        aPosAry.mnDestWidth = ImplLogicWidthToDevicePixel( rDestSize.Width() );
        aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

        // spiegeln via Koordinaten wollen wir nicht
        const ULONG nMirrFlags = ImplAdjustTwoRect( aPosAry, pImpBmp->ImplGetSize() );

        // check if output is necessary
        if( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
        {
#ifndef REMOTE_APPSERVER

            if( nMirrFlags )
            {
                Bitmap aTmp( rBitmap );
                aTmp.Mirror( nMirrFlags );
                mpGraphics->DrawMask( &aPosAry, *aTmp.ImplGetImpBitmap()->ImplGetSalBitmap(),
                                      ImplColorToSal( rMaskColor ) );
            }
            else
                mpGraphics->DrawMask( &aPosAry, *pImpBmp->ImplGetSalBitmap(),
                                      ImplColorToSal( rMaskColor ) );

#else

            if( nMirrFlags )
            {
                Bitmap aTmp( rBitmap );
                aTmp.Mirror( nMirrFlags );
                aTmp.ImplDrawRemoteMask( this,
                    Point( aPosAry.mnSrcX, aPosAry.mnSrcY ),
                    Size( aPosAry.mnSrcWidth, aPosAry.mnSrcHeight ),
                    Point( aPosAry.mnDestX, aPosAry.mnDestY ),
                    Size( aPosAry.mnDestWidth, aPosAry.mnDestHeight ),
                    rMaskColor );
            }
            else
                rBitmap.ImplDrawRemoteMask( this,
                    Point( aPosAry.mnSrcX, aPosAry.mnSrcY ),
                    Size( aPosAry.mnSrcWidth, aPosAry.mnSrcHeight ),
                    Point( aPosAry.mnDestX, aPosAry.mnDestY ),
                    Size( aPosAry.mnDestWidth, aPosAry.mnDestHeight ),
                    rMaskColor );

#endif
        }
    }
}

// ------------------------------------------------------------------

Bitmap OutputDevice::GetBitmap( const Point& rSrcPt, const Size& rSize ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Bitmap  aBmp;
    long    nX = ImplLogicXToDevicePixel( rSrcPt.X() );
    long    nY = ImplLogicYToDevicePixel( rSrcPt.Y() );
    long    nWidth = ImplLogicWidthToDevicePixel( rSize.Width() );
    long    nHeight = ImplLogicHeightToDevicePixel( rSize.Height() );

#ifndef REMOTE_APPSERVER
    if ( mpGraphics || ( (OutputDevice*) this )->ImplGetGraphics() )
#endif
    {
        if ( nWidth && nHeight )
        {
#ifndef REMOTE_APPSERVER
            Rectangle   aRect( Point( nX, nY ), Size( nWidth, nHeight ) );
            BOOL        bClipped = FALSE;

            // X-Koordinate ausserhalb des Bereichs?
            if ( nX < mnOutOffX )
            {
                nWidth -= ( mnOutOffX - nX );
                nX = mnOutOffX;
                bClipped = TRUE;
            }

            // Y-Koordinate ausserhalb des Bereichs?
            if ( nY < mnOutOffY )
            {
                nHeight -= ( mnOutOffY - nY );
                nY = mnOutOffY;
                bClipped = TRUE;
            }

            // Breite ausserhalb des Bereichs?
            if ( (nWidth + nX) > (mnOutWidth + mnOutOffX) )
            {
                nWidth  = mnOutOffX + mnOutWidth - nX;
                bClipped = TRUE;
            }

            // Hoehe ausserhalb des Bereichs?
            if ( (nHeight + nY) > (mnOutHeight + mnOutOffY) )
            {
                nHeight = mnOutOffY + mnOutHeight - nY;
                bClipped = TRUE;
            }

            if ( bClipped )
            {
                // Falls auf den sichtbaren Bereich geclipped wurde,
                // muessen wir eine Bitmap in der rchtigen Groesse
                // erzeugen, in die die geclippte Bitmap an die angepasste
                // Position kopiert wird
                VirtualDevice aVDev( *this );

                if ( aVDev.SetOutputSizePixel( aRect.GetSize() ) )
                {
                    if ( ((OutputDevice*)&aVDev)->mpGraphics || ((OutputDevice*)&aVDev)->ImplGetGraphics() )
                    {
                        TwoRect aPosAry;

                        aPosAry.mnSrcX = nX;
                        aPosAry.mnSrcY = nY;
                        aPosAry.mnSrcWidth = nWidth;
                        aPosAry.mnSrcHeight = nHeight;
                        aPosAry.mnDestX = ( aRect.Left() < mnOutOffX ) ? ( mnOutOffX - aRect.Left() ) : 0L;
                        aPosAry.mnDestY = ( aRect.Top() < mnOutOffY ) ? ( mnOutOffY - aRect.Top() ) : 0L;
                        aPosAry.mnDestWidth = nWidth;
                        aPosAry.mnDestHeight = nHeight;

                        if ( (nWidth > 0) && (nHeight > 0) )
                            (((OutputDevice*)&aVDev)->mpGraphics)->CopyBits( &aPosAry, mpGraphics );

                        aBmp = aVDev.GetBitmap( Point(), aVDev.GetOutputSizePixel() );
                     }
                     else
                        bClipped = FALSE;
                }
                else
                    bClipped = FALSE;
            }

            if ( !bClipped )
            {
                SalBitmap* pSalBmp = mpGraphics->GetBitmap( nX, nY, nWidth, nHeight );

                if( pSalBmp )
                {
                    ImpBitmap* pImpBmp = new ImpBitmap;
                    pImpBmp->ImplSetSalBitmap( pSalBmp );
                    aBmp.ImplSetImpBitmap( pImpBmp );
                }
            }
#else
            aBmp.ImplGetRemoteBmp( (OutputDevice*) this, Point( nX, nY ), Size( nWidth, nHeight ) );
#endif
        }
    }

    return aBmp;
}

// ------------------------------------------------------------------

void OutputDevice::ImplGetFrameBitmap( const Point& rDestPt, const Size& rSize,
                                       Bitmap& rBitmap ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    BOOL bOldMap = mbMap;
    ((OutputDevice*)this)->mbMap = FALSE;
    rBitmap = GetBitmap( rDestPt, rSize );
    ((OutputDevice*)this)->mbMap = bOldMap;
}

// ------------------------------------------------------------------

Color OutputDevice::GetPixel( const Point& rPt ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor;

#ifndef REMOTE_APPSERVER
    if ( mpGraphics || ((OutputDevice*)this)->ImplGetGraphics() )
    {
        if ( mbInitClipRegion )
            ((OutputDevice*)this)->ImplInitClipRegion();

        if ( !mbOutputClipped )
        {
            const long      nX = ImplLogicXToDevicePixel( rPt.X() );
            const long      nY = ImplLogicYToDevicePixel( rPt.Y() );
            const SalColor  aSalCol = mpGraphics->GetPixel( nX, nY );
            aColor.SetRed( SALCOLOR_RED( aSalCol ) );
            aColor.SetGreen( SALCOLOR_GREEN( aSalCol ) );
            aColor.SetBlue( SALCOLOR_BLUE( aSalCol ) );
        }
    }
#else // REMOTE_APPSERVER
    ImplServerGraphics* pGraphics = ( (OutputDevice*) this )->ImplGetServerGraphics();
    if( pGraphics )
    {
        const long nX = ImplLogicXToDevicePixel( rPt.X() );
        const long nY = ImplLogicYToDevicePixel( rPt.Y() );
        aColor = pGraphics->GetPixel( Point( nX, nY ) );
    }
#endif // REMOTE_APPSERVER

    return aColor;
}

// ------------------------------------------------------------------

Color* OutputDevice::GetPixel( const Polygon& rPts ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color*          pColors = NULL;
    const USHORT    nSize = rPts.GetSize();

    if( nSize )
    {
#ifndef REMOTE_APPSERVER
        if ( mpGraphics || ((OutputDevice*)this)->ImplGetGraphics() )
        {
            if ( mbInitClipRegion )
                ((OutputDevice*)this)->ImplInitClipRegion();

            if ( !mbOutputClipped )
            {
                pColors = new Color[ nSize ];

                for( USHORT i = 0; i < nSize; i++ )
                {
                    Color&          rCol = pColors[ i ];
                    const Point&    rPt = rPts[ i ];
                    const SalColor  aSalCol( mpGraphics->GetPixel( ImplLogicXToDevicePixel( rPt.X() ),
                                                                   ImplLogicYToDevicePixel( rPt.Y() ) ) );

                    rCol.SetRed( SALCOLOR_RED( aSalCol ) );
                    rCol.SetGreen( SALCOLOR_GREEN( aSalCol ) );
                    rCol.SetBlue( SALCOLOR_BLUE( aSalCol ) );
                }
            }
        }
#else // REMOTE_APPSERVER
        ImplServerGraphics* pGraphics = ( (OutputDevice*) this )->ImplGetServerGraphics();
        if( pGraphics )
        {
            pColors = pGraphics->GetPixel( ImplLogicToDevicePixel( rPts ) );
        }
#endif // REMOTE_APPSERVER
    }

    return pColors;
}

// -----------------------------------------------------------------------

void OutputDevice::DrawPixel( const Point& rPt )
{
    DBG_TRACE( "OutputDevice::DrawPixel()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPointAction( rPt ) );

    if ( !IsDeviceOutputNecessary() || !mbLineColor )
        return;

#ifndef REMOTE_APPSERVER
    Point aPt = ImplLogicToDevicePixel( rPt );

    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    mpGraphics->DrawPixel( aPt.X(), aPt.Y() );
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        pGraphics->DrawPixel( ImplLogicToDevicePixel( rPt ) );
    }
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::DrawPixel( const Point& rPt, const Color& rColor )
{
    DBG_TRACE( "OutputDevice::DrawPixel()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );

    if( mnDrawMode & ( DRAWMODE_BLACKLINE | DRAWMODE_WHITELINE |
                       DRAWMODE_GRAYLINE | DRAWMODE_GHOSTEDLINE ) )
    {
        if( !ImplIsColorTransparent( aColor ) )
        {
            if( mnDrawMode & DRAWMODE_BLACKLINE )
            {
                aColor = Color( COL_BLACK );
            }
            else if( mnDrawMode & DRAWMODE_WHITELINE )
            {
                aColor = Color( COL_WHITE );
            }
            else if( mnDrawMode & DRAWMODE_GRAYLINE )
            {
                const UINT8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }

            if( mnDrawMode & DRAWMODE_GHOSTEDLINE )
            {
                aColor = Color( ( aColor.GetRed() >> 1 ) | 0x80,
                                ( aColor.GetGreen() >> 1 ) | 0x80,
                                ( aColor.GetBlue() >> 1 ) | 0x80 );
            }
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPixelAction( rPt, aColor ) );

    if ( !IsDeviceOutputNecessary() || ImplIsColorTransparent( aColor ) )
        return;

#ifndef REMOTE_APPSERVER
    Point aPt = ImplLogicToDevicePixel( rPt );

    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    mpGraphics->DrawPixel( aPt.X(), aPt.Y(), ImplColorToSal( aColor ) );
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
        pGraphics->DrawPixel( ImplLogicToDevicePixel( rPt ), aColor );
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::DrawPixel( const Polygon& rPts, const Color* pColors )
{
    if ( !pColors )
        DrawPixel( rPts, GetLineColor() );
    else
    {
        DBG_TRACE( "OutputDevice::DrawPixel()" );
        DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
        DBG_ASSERT( pColors, "OutputDevice::DrawPixel: No color array specified" );

        const USHORT nSize = rPts.GetSize();

        if ( nSize )
        {
            if ( mpMetaFile )
                for ( USHORT i = 0; i < nSize; i++ )
                    mpMetaFile->AddAction( new MetaPixelAction( rPts[ i ], pColors[ i ] ) );

            if ( !IsDeviceOutputNecessary() )
                return;

#ifndef REMOTE_APPSERVER
            // we need a graphics
            if ( mpGraphics || ImplGetGraphics() )
            {
                if ( mbInitClipRegion )
                    ImplInitClipRegion();

                if ( mbOutputClipped )
                    return;

                for ( USHORT i = 0; i < nSize; i++ )
                {
                    const Point aPt( ImplLogicToDevicePixel( rPts[ i ] ) );
                    mpGraphics->DrawPixel( aPt.X(), aPt.Y(), ImplColorToSal( pColors[ i ] ) );
                }
            }
#else
            ImplServerGraphics* pGraphics = ImplGetServerGraphics();
            if ( pGraphics )
            {
                pGraphics->DrawPixel( ImplLogicToDevicePixel( rPts ), pColors );
            }
#endif
        }
    }
}

// -----------------------------------------------------------------------

void OutputDevice::DrawPixel( const Polygon& rPts, const Color& rColor )
{
    if( rColor != COL_TRANSPARENT )
    {
        const USHORT    nSize = rPts.GetSize();
        Color*          pColArray = new Color[ nSize ];

        for( USHORT i = 0; i < nSize; i++ )
            pColArray[ i ] = rColor;

        DrawPixel( rPts, pColArray );
        delete[] pColArray;
    }
}

// ------------------------------------------------------------------------

void OutputDevice::ImplDrawAlpha( const Bitmap& rBmp, const AlphaMask& rAlpha,
                                  const Point& rDestPt, const Size& rDestSize,
                                  const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    Point       aPt;
    Point       aOutPt( LogicToPixel( rDestPt ) );
    Size        aOutSz( LogicToPixel( rDestSize ) );
    Rectangle   aDstRect( aPt, GetOutputSizePixel() );
    const BOOL  bHMirr = aOutSz.Width() < 0, bVMirr = aOutSz.Height() < 0;

    if( OUTDEV_WINDOW == meOutDevType )
    {
        const Region aPaintRgn( ( (Window*) this )->GetPaintRegion() );

        if( !aPaintRgn.IsNull() )
            aDstRect.Intersection( LogicToPixel( aPaintRgn.GetBoundRect() ) );
    }

    if( bHMirr )
    {
        aOutSz.Width() = -aOutSz.Width();
        aOutPt.X() -= ( aOutSz.Width() - 1L );
    }

    if( bVMirr )
    {
        aOutSz.Height() = -aOutSz.Height();
        aOutPt.Y() -= ( aOutSz.Height() - 1L );
    }

    if( !aDstRect.Intersection( Rectangle( aOutPt, aOutSz ) ).IsEmpty() )
    {
        Rectangle aBmpRect( aPt, rBmp.GetSizePixel() );

        if( !aBmpRect.Intersection( Rectangle( rSrcPtPixel, rSrcSizePixel ) ).IsEmpty() )
        {
            GDIMetaFile*    pOldMetaFile = mpMetaFile; mpMetaFile = NULL;
            const BOOL      bOldMap = mbMap; mbMap = FALSE;
            Bitmap          aBmp( GetBitmap( aDstRect.TopLeft(), aDstRect.GetSize() ) );
            BitmapColor     aDstCol;
            const long      nSrcWidth = aBmpRect.GetWidth(), nSrcHeight = aBmpRect.GetHeight();
            const long      nDstWidth = aDstRect.GetWidth(), nDstHeight = aDstRect.GetHeight();
            const long      nOutWidth = aOutSz.Width(), nOutHeight = aOutSz.Height();
            const long      nOffX = aDstRect.Left() - aOutPt.X(), nOffY = aDstRect.Top() - aOutPt.Y();
            long            nX, nOutX, nY, nOutY, nMirrOffX, nMirrOffY;
            long*           pMapX = new long[ nDstWidth ];
            long*           pMapY = new long[ nDstHeight ];

            // create horizontal mapping table
            if( bHMirr )
                nMirrOffX = ( aBmpRect.Left() << 1 ) + nSrcWidth - 1;

            for( nX = 0L, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
            {
                pMapX[ nX ] = aBmpRect.Left() + nOutX * nSrcWidth / nOutWidth;

                if( bHMirr )
                    pMapX[ nX ] = nMirrOffX - pMapX[ nX ];
            }

            // create vertical mapping table
            if( bVMirr )
                nMirrOffY = ( aBmpRect.Top() << 1 ) + nSrcHeight - 1;

            for( nY = 0L, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
            {
                pMapY[ nY ] = aBmpRect.Top() + nOutY * nSrcHeight / nOutHeight;

                if( bVMirr )
                    pMapY[ nY ] = nMirrOffY - pMapY[ nY ];
            }

            if( GetBitCount() <= 8 )
            {
                Bitmap              aDither( aBmp.GetSizePixel(), 8 );
                BitmapColor         aIndex( 0 );
                BitmapReadAccess*   pP = ( (Bitmap&) rBmp ).AcquireReadAccess();
                BitmapReadAccess*   pA = ( (AlphaMask&) rAlpha ).AcquireReadAccess();
                BitmapReadAccess*   pB = aBmp.AcquireReadAccess();
                BitmapWriteAccess*  pW = aDither.AcquireWriteAccess();

                if( pB && pP && pA && pW )
                {
                    for( nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
                    {
                        const long nMapY = pMapY[ nY ];
                        const long nModY = ( nOutY & 0x0FL ) << 4L;

                        for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                        {
                            const long  nMapX = pMapX[ nX ];
                            const ULONG nD = nVCLDitherLut[ nModY | ( nOutX & 0x0FL ) ];

                            aDstCol = pB->GetColor( nY, nX );
                            aDstCol.Merge( pP->GetColor( nMapY, nMapX ), (BYTE) pA->GetPixel( nMapY, nMapX ) );
                            aIndex.SetIndex( (BYTE) ( nVCLRLut[ ( nVCLLut[ aDstCol.GetRed() ] + nD ) >> 16UL ] +
                                                      nVCLGLut[ ( nVCLLut[ aDstCol.GetGreen() ] + nD ) >> 16UL ] +
                                                      nVCLBLut[ ( nVCLLut[ aDstCol.GetBlue() ] + nD ) >> 16UL ] ) );
                            pW->SetPixel( nY, nX, aIndex );
                        }
                    }
                }

                ( (Bitmap&) rBmp ).ReleaseAccess( pP );
                ( (AlphaMask&) rAlpha ).ReleaseAccess( pA );
                aBmp.ReleaseAccess( pB );
                aDither.ReleaseAccess( pW );
                DrawBitmap( aDstRect.TopLeft(), aDither );
            }
            else
            {
                BitmapReadAccess*   pP = ( (Bitmap&) rBmp ).AcquireReadAccess();
                BitmapReadAccess*   pA = ( (AlphaMask&) rAlpha ).AcquireReadAccess();
                BitmapWriteAccess*  pB = aBmp.AcquireWriteAccess();

                if( pP && pA && pB )
                {
                    if( pA->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                    {
                        switch( pP->GetScanlineFormat() )
                        {
                            case( BMP_FORMAT_8BIT_PAL ):
                            {
                                for( nY = 0; nY < nDstHeight; nY++ )
                                {
                                    const long  nMapY = pMapY[ nY ];
                                    Scanline    pPScan = pP->GetScanline( nMapY );
                                    Scanline    pAScan = pA->GetScanline( nMapY );

                                    for( nX = 0; nX < nDstWidth; nX++ )
                                    {
                                        const long nMapX = pMapX[ nX ];
                                        aDstCol = pB->GetPixel( nY, nX );
                                        pB->SetPixel( nY, nX, aDstCol.Merge( pP->GetPaletteColor( pPScan[ nMapX ] ),
                                                                             pAScan[ nMapX ] ) );
                                    }
                                }
                            }
                            break;

                            case( BMP_FORMAT_24BIT_TC_BGR ):
                            {
                                for( nY = 0; nY < nDstHeight; nY++ )
                                {
                                    const long  nMapY = pMapY[ nY ];
                                    Scanline    pPScan = pP->GetScanline( nMapY );
                                    Scanline    pAScan = pA->GetScanline( nMapY );

                                    for( nX = 0; nX < nDstWidth; nX++ )
                                    {
                                        const long  nMapX = pMapX[ nX ];
                                        Scanline    pTmp = pPScan + nMapX * 3;

                                        aDstCol = pB->GetPixel( nY, nX );
                                        pB->SetPixel( nY, nX, aDstCol.Merge( pTmp[ 2 ], pTmp[ 1 ], pTmp[ 0 ],
                                                                             pAScan[ nMapX ] ) );
                                    }
                                }
                            }
                            break;

                            case( BMP_FORMAT_24BIT_TC_RGB ):
                            {
                                for( nY = 0; nY < nDstHeight; nY++ )
                                {
                                    const long  nMapY = pMapY[ nY ];
                                    Scanline    pPScan = pP->GetScanline( nMapY );
                                    Scanline    pAScan = pA->GetScanline( nMapY );

                                    for( nX = 0; nX < nDstWidth; nX++ )
                                    {
                                        const long  nMapX = pMapX[ nX ];
                                        Scanline    pTmp = pPScan + nMapX * 3;

                                        aDstCol = pB->GetPixel( nY, nX );
                                        pB->SetPixel( nY, nX, aDstCol.Merge( pTmp[ 0 ], pTmp[ 1 ], pTmp[ 2 ],
                                                                             pAScan[ nMapX ] ) );
                                    }
                                }
                            }
                            break;

                            default:
                            {
                                for( nY = 0; nY < nDstHeight; nY++ )
                                {
                                    const long  nMapY = pMapY[ nY ];
                                    Scanline    pAScan = pA->GetScanline( nMapY );

                                    for( nX = 0; nX < nDstWidth; nX++ )
                                    {
                                        const long nMapX = pMapX[ nX ];
                                        aDstCol = pB->GetPixel( nY, nX );
                                        pB->SetPixel( nY, nX, aDstCol.Merge( pP->GetColor( nMapY, nMapX ),
                                                                             pAScan[ nMapX ] ) );
                                    }
                                }
                            }
                            break;
                        }
                    }
                    else
                    {
                        for( nY = 0; nY < nDstHeight; nY++ )
                        {
                            const long nMapY = pMapY[ nY ];

                            for( nX = 0; nX < nDstWidth; nX++ )
                            {
                                const long nMapX = pMapX[ nX ];
                                aDstCol = pB->GetPixel( nY, nX );
                                pB->SetPixel( nY, nX, aDstCol.Merge( pP->GetColor( nMapY, nMapX ),
                                                                     (BYTE) pA->GetPixel( nMapY, nMapX ) ) );
                            }
                        }
                    }
                }

                ( (Bitmap&) rBmp ).ReleaseAccess( pP );
                ( (AlphaMask&) rAlpha ).ReleaseAccess( pA );
                aBmp.ReleaseAccess( pB );
                DrawBitmap( aDstRect.TopLeft(), aBmp );
            }

            delete[] pMapX;
            delete[] pMapY;
            mbMap = bOldMap;
            mpMetaFile = pOldMetaFile;
        }
    }
}

// ------------------------------------------------------------------------

static Pair* ImplGetMap( long nFromSize, long nToSize )
{
    DBG_ASSERT( nFromSize && nToSize, "ImplGetMap(): Invalid size!" );

    Pair*           pMap = new Pair[ nFromSize ];
    const double    fSize = (double) nToSize / nFromSize;
    double          fRealSum = 0.0;
    const long      nLastToPos = nToSize - 1L;
    long            nErrSum = 0L, nPos = 0L, nSize = 0L;

    for( long i = 0L; i < nFromSize; i++ )
    {
        nPos = nPos + nSize;
        nSize = Max( FRound( fSize - ( nErrSum - fRealSum ) ), 0L );

        nErrSum += nSize;
        fRealSum += fSize;

        pMap[ i ].A() = nPos = Min( nPos, nLastToPos );
        pMap[ i ].B() = Min( nPos + Max( nSize, 1L ) - 1L, nLastToPos );
    }

    return pMap;
}

// ------------------------------------------------------------------------

static BOOL ImplCreateBandBitmaps( BitmapReadAccess* pPAcc, BitmapReadAccess* pMAcc,
                                   long* pMapX, long* pMapY,
                                   long nDstWidth, long nDstY1, long nDstY2,
                                   Bitmap& rPaint, Bitmap& rMask )
{
    const Size  aSz( nDstWidth, nDstY2 - nDstY1 + 1 );
    BOOL        bRet = FALSE;

    rPaint = Bitmap( aSz, pPAcc->GetBitCount(), pPAcc->HasPalette() ? &pPAcc->GetPalette() : NULL );
    rMask = Bitmap( aSz, pMAcc->GetBitCount(), pMAcc->HasPalette() ? &pMAcc->GetPalette() : NULL );

    BitmapWriteAccess* pWPAcc = rPaint.AcquireWriteAccess();
    BitmapWriteAccess* pWMAcc = rMask.AcquireWriteAccess();

    if( pWPAcc && pWMAcc )
    {
        const long  nWidth = pWPAcc->Width();
        const long  nHeight = pWPAcc->Width();
        const long  nPScanSize = pWPAcc->GetScanlineSize();
        const long  nMScanSize = pWMAcc->GetScanlineSize();
        long        nY = 0, nScanY = nDstY1;

        while( nScanY <= nDstY2 )
        {
            const long nMapY = pMapY[ nScanY ];

            for( long nX = 0L; nX < nWidth; nX++ )
            {
                const long nMapX = pMapX[ nX ];
                pWPAcc->SetPixel( nY, nX, pPAcc->GetPixel( nMapY, nMapX ) );
                pWMAcc->SetPixel( nY, nX, pMAcc->GetPixel( nMapY, nMapX ) );
            }

            while( ( nScanY < nDstY2 ) && ( pMapY[ nScanY + 1 ] == nMapY ) )
            {
                HMEMCPY( pWPAcc->GetScanline( nY + 1L ), pWPAcc->GetScanline( nY ), nPScanSize );
                HMEMCPY( pWMAcc->GetScanline( nY + 1L ), pWMAcc->GetScanline( nY ), nMScanSize );
                nY++, nScanY++;
            }

            nY++, nScanY++;
        }

        bRet = TRUE;
    }

    if( pWPAcc )
        rPaint.ReleaseAccess( pWPAcc );

    if( pWMAcc )
        rMask.ReleaseAccess( pWMAcc );

    return bRet;
}

// ------------------------------------------------------------------------

void OutputDevice::ImplPrintTransparent( const Bitmap& rBmp, const Bitmap& rMask,
                                         const Point& rDestPt, const Size& rDestSize,
                                         const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    Point       aPt;
    Point       aDestPt( LogicToPixel( rDestPt ) );
    Size        aDestSz( LogicToPixel( rDestSize ) );
    Rectangle   aSrcRect( rSrcPtPixel, rSrcSizePixel );

    aSrcRect.Justify();

    if( !!rBmp && aSrcRect.GetWidth() && aSrcRect.GetHeight() && aDestSz.Width() && aDestSz.Height() )
    {
        ULONG   nMirrFlags = 0UL;
        Bitmap  aPaint( rBmp );
        Bitmap  aMask( rMask );
        Region  aDstRgn;
        BOOL    bMask = !!aMask;

        if( bMask && ( aMask.GetBitCount() > 1 ) )
            aMask.Convert( BMP_CONVERSION_1BIT_THRESHOLD );

        // mirrored horizontically
        if( aDestSz.Width() < 0L )
        {
            aDestSz.Width() = -aDestSz.Width();
            aDestPt.X() -= ( aDestSz.Width() - 1L );
            nMirrFlags |= BMP_MIRROR_HORZ;
        }

        // mirrored vertically
        if( aDestSz.Height() < 0L )
        {
            aDestSz.Height() = -aDestSz.Height();
            aDestPt.Y() -= ( aDestSz.Height() - 1L );
            nMirrFlags |= BMP_MIRROR_VERT;
        }

        // source cropped?
        if( aSrcRect != Rectangle( aPt, aPaint.GetSizePixel() ) )
        {
            aPaint.Crop( aSrcRect );
            if( bMask )
                aMask.Crop( aSrcRect );
        }

        // destination mirrored
        if( nMirrFlags )
        {
            aPaint.Mirror( nMirrFlags );
            if( bMask )
                aMask.Mirror( nMirrFlags );
        }

        const Rectangle aDstRect( aDestPt, aDestSz );

        // create destination region
        if( mbClipRegion && !maRegion.IsEmpty() )
        {
            aDstRgn = maRegion;
            aDstRgn.Intersect( aDstRect );
        }
        else
            aDstRgn = aDstRect;

        aDstRgn.Move( -aDstRect.Left(), -aDstRect.Top() );

        // we always want to have a mask
        if( !bMask )
        {
            aMask = Bitmap( aSrcRect.GetSize(), 1 );
            aMask.Erase( Color( COL_BLACK ) );
        }

        BitmapReadAccess* pPAcc = aPaint.AcquireReadAccess();
        BitmapReadAccess* pMAcc = aMask.AcquireReadAccess();

        if( pPAcc && pMAcc )
        {
            const long      nWidth = aDestSz.Width();
            const long      nHeight = aDestSz.Height();
            const long      nWidth1 = nWidth - 1;
            const long      nHeight1 = nHeight - 1;
            const long      nOldWidth1 = aSrcRect.GetWidth() - 1;
            const long      nOldHeight1 = aSrcRect.GetHeight() - 1;
            const long      nScanByteCount = Max( nWidth * aPaint.GetBitCount() / 8L, 1L );
            const long      nBandHeight = BAND_MAX_SIZE / nScanByteCount + 1;
            long*           pMapX = new long[ nWidth ];
            long*           pMapY = new long[ nHeight ];
            long            nX, nY;
            long            nBandY1, nBandY2;
            GDIMetaFile*    pOldMetaFile = mpMetaFile;
            const BOOL      bOldMap = mbMap;

            mpMetaFile = NULL;
            Push( PUSH_CLIPREGION );
            SetClipRegion();
            mbMap = FALSE;

            // create mapping tables
            for( nX = 0L; nX < nWidth; nX++ )
                pMapX[ nX ] = nWidth1 ? ( nX * nOldWidth1 / nWidth1 ) : 0;

            for( nY = 0L; nY < nHeight; nY++ )
                pMapY[ nY ] = nHeight1 ? ( nY * nOldHeight1 / nHeight1 ) : 0;

            // process bands
            for( nBandY1 = 0, nBandY2 = nBandHeight; nBandY1 < nHeight; nBandY1 += nBandHeight, nBandY2 += nBandHeight )
            {
                Bitmap aWorkPaint, aWorkMask;

                // don't walk over bounds
                if( nBandY2 > nHeight1 )
                    nBandY2 = nHeight1;

                if( ImplCreateBandBitmaps( pPAcc, pMAcc, pMapX, pMapY, nWidth, nBandY1, nBandY2, aWorkPaint, aWorkMask ) )
                {
                    Region aWorkRgn( aDstRgn );
                    aWorkRgn.Move( 0, -nBandY1 );
                    aWorkRgn.Intersect( aWorkMask.CreateRegion( COL_BLACK, Rectangle( aPt, aWorkMask.GetSizePixel() ) ) );

                    ImplRegionInfo  aInfo;
                    long            nWorkX, nWorkY, nWorkWidth, nWorkHeight;
                    BOOL            bRgnRect = aWorkRgn.ImplGetFirstRect( aInfo, nWorkX, nWorkY,
                                                                          nWorkWidth, nWorkHeight );

                    while( bRgnRect )
                    {
                        Bitmap      aCropBmp( aWorkPaint );
                        const Point aOutPt( nWorkX + aDestPt.X(), nWorkY + nBandY1 + aDestPt.Y() );
                        const Size  aOutSz( nWorkWidth, nWorkHeight );
                        const Size  aOutSz1( nWorkWidth + 1, nWorkHeight + 1 );

                        aCropBmp.Crop( Rectangle( Point( nWorkX, nWorkY ), aOutSz ) );
                        ImplDrawBitmap( aOutPt, aOutSz1, Point(), aOutSz, aCropBmp, META_BMPSCALE_ACTION );
                        bRgnRect = aWorkRgn.ImplGetNextRect( aInfo, nWorkX, nWorkY, nWorkWidth, nWorkHeight );
                    }
                }
            }

            delete[] pMapX;
            delete[] pMapY;
            mbMap = bOldMap;
            Pop();
            mpMetaFile = pOldMetaFile;
        }

        if( pPAcc )
            aPaint.ReleaseAccess( pPAcc );

        if( pMAcc )
            aMask.ReleaseAccess( pMAcc );
    }
}

// ------------------------------------------------------------------------

void OutputDevice::ImplPrintMask( const Bitmap& rMask, const Color& rMaskColor,
                                  const Point& rDestPt, const Size& rDestSize,
                                  const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
#ifndef REMOTE_APPSERVER

    Point       aPt;
    Point       aDestPt( LogicToPixel( rDestPt ) );
    Size        aDestSz( LogicToPixel( rDestSize ) );
    Rectangle   aSrcRect( rSrcPtPixel, rSrcSizePixel );

    aSrcRect.Justify();

    if( !!rMask &&
        aSrcRect.GetWidth() && aSrcRect.GetHeight() &&
        aDestSz.Width() && aDestSz.Height() )
    {
        ULONG   nMirrFlags = 0UL;
        Bitmap  aMask( rMask );
        Region  aRegion;

        if( aMask.GetBitCount() > 1 )
            aMask.Convert( BMP_CONVERSION_1BIT_THRESHOLD );

        if( aDestSz.Width() < 0L )
        {
            aDestSz.Width() = -aDestSz.Width();
            aDestPt.X() -= ( aDestSz.Width() - 1L );
            nMirrFlags |= BMP_MIRROR_HORZ;
        }

        if( aDestSz.Height() < 0L )
        {
            aDestSz.Height() = -aDestSz.Height();
            aDestPt.Y() -= ( aDestSz.Height() - 1L );
            nMirrFlags |= BMP_MIRROR_VERT;
        }

        // source cropped?
        if( aSrcRect != Rectangle( aPt, aMask.GetSizePixel() ) )
            aMask.Crop( aSrcRect );

        // destination mirrored
        if( nMirrFlags )
            aMask.Mirror( nMirrFlags );

        aRegion = aMask.CreateRegion( COL_BLACK, Rectangle( Point(), aMask.GetSizePixel() ) );

        ImplRegionInfo      aInfo;
        const Size          aSrcSz( aMask.GetSizePixel() );
        long                nSrcX, nSrcY, nSrcWidth, nSrcHeight;
        long                nDstX, nDstY, nDstWidth, nDstHeight;
        GDIMetaFile*        pOldMetaFile = mpMetaFile;
        Pair*               pMapX = ImplGetMap( aSrcSz.Width(), aDestSz.Width() );
        Pair*               pMapY = ImplGetMap( aSrcSz.Height(), aDestSz.Height() );
        BOOL                bOldMap = mbMap;
        BOOL                bRegionRect = aRegion.ImplGetFirstRect( aInfo, nSrcX, nSrcY, nSrcWidth, nSrcHeight );

        mpMetaFile = NULL;
        mbMap = FALSE;
        Push( PUSH_FILLCOLOR | PUSH_LINECOLOR );
        SetLineColor( rMaskColor );
        SetFillColor( rMaskColor );
        ImplInitLineColor();
        ImplInitFillColor();

        while( bRegionRect )
        {
            nDstX = pMapX[ nSrcX ].A();
            nDstY = pMapY[ nSrcY ].A();
            nDstWidth = pMapX[ nSrcX + nSrcWidth - 1L ].B() - nDstX + 1L;
            nDstHeight = pMapY[ nSrcY + nSrcHeight - 1L ].B() - nDstY + 1L;
            mpGraphics->DrawRect( nDstX + aDestPt.X(), nDstY + aDestPt.Y(), nDstWidth, nDstHeight );
            bRegionRect = aRegion.ImplGetNextRect( aInfo, nSrcX, nSrcY, nSrcWidth, nSrcHeight );
        }

        Pop();
        delete[] pMapX;
        delete[] pMapY;
        mbMap = bOldMap;
        mpMetaFile = pOldMetaFile;
    }

#endif
}
