/*************************************************************************
 *
 *  $RCSfile: outdev2.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-24 15:24:50 $
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
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_IMAGE_H
#include <image.h>
#endif
#ifndef _SV_IMAGE_HXX
#include <image.hxx>
#endif

#define BAND_MAX_SIZE 512000

// =======================================================================

DBG_NAMEEX( OutputDevice );

// =======================================================================

// -----------
// - Defines -
// -----------

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

#define TwoRect     SalTwoRect

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
    SalGraphics*        pGraphics2;

    if ( pPosAry->mnSrcWidth && pPosAry->mnSrcHeight && pPosAry->mnDestWidth && pPosAry->mnDestHeight )
    {
        if ( this == pSrcDev )
            pGraphics2 = NULL;
        else
        {
            if ( (GetOutDevType() != pSrcDev->GetOutDevType()) ||
                 (GetOutDevType() != OUTDEV_WINDOW) )
            {
                if ( !pSrcDev->mpGraphics )
                {
                    if ( !((OutputDevice*)pSrcDev)->ImplGetGraphics() )
                        return;
                }
                pGraphics2 = pSrcDev->mpGraphics;
            }
            else
            {
                if ( ((Window*)this)->mpFrameWindow == ((Window*)pSrcDev)->mpFrameWindow )
                    pGraphics2 = NULL;
                else
                {
                    if ( !pSrcDev->mpGraphics )
                    {
                        if ( !((OutputDevice*)pSrcDev)->ImplGetGraphics() )
                            return;
                    }
                    pGraphics2 = pSrcDev->mpGraphics;

                    if ( !mpGraphics )
                    {
                        if ( !ImplGetGraphics() )
                            return;
                    }
                    DBG_ASSERT( mpGraphics && pSrcDev->mpGraphics,
                                "OutputDevice::DrawOutDev(): We need more than one Graphics" );
                }
            }
        }

        // #102532# Offset only has to be pseudo window offset
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

            // --- RTL --- if this is no window, but pSrcDev is a window
            // mirroring may be required
            // because only windows have a SalGraphicsLayout
            // mirroring is performed here
            if( (GetOutDevType() != OUTDEV_WINDOW) && pGraphics2 && (pGraphics2->GetLayout() & SAL_LAYOUT_BIDI_RTL) )
            {
                SalTwoRect pPosAry2 = *pPosAry;
                pGraphics2->mirror( pPosAry2.mnSrcX, pPosAry2.mnSrcWidth, pSrcDev );
                mpGraphics->CopyBits( &pPosAry2, pGraphics2, this, pSrcDev );
            }
            else
                mpGraphics->CopyBits( pPosAry, pGraphics2, this, pSrcDev );
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

    if( ImplIsRecordLayout() )
        return;

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

            mpGraphics->CopyBits( &aPosAry, NULL, this, NULL );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawOutDev( rDestPt, rDestSize, rSrcPt, rSrcSize );
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

    if ( (meOutDevType == OUTDEV_PRINTER) || (rOutDev.meOutDevType == OUTDEV_PRINTER) || ImplIsRecordLayout() )
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

    if( mpAlphaVDev )
    {
        if( rOutDev.mpAlphaVDev )
        {
            // alpha-blend source over destination
            DrawBitmapEx( rDestPt, rDestSize, rOutDev.GetBitmapEx(rSrcPt, rSrcSize) );

            // This would be mode SOURCE:
            // copy source alpha channel to our alpha channel
            //mpAlphaVDev->DrawOutDev( rDestPt, rDestSize, rSrcPt, rSrcSize, *rOutDev.mpAlphaVDev );
        }
        else
        {
            ImplDrawOutDevDirect( &rOutDev, &aPosAry );

            // make destination rectangle opaque - source has no alpha
            mpAlphaVDev->DrawRect( Rectangle(rDestPt, rDestSize) );
        }
    }
    else
    {
        if( rOutDev.mpAlphaVDev )
        {
            // alpha-blend source over destination
            DrawBitmapEx( rDestPt, rDestSize, rOutDev.GetBitmapEx(rSrcPt, rSrcSize) );
        }
        else
        {
            // no alpha at all, neither in source nor destination device
            ImplDrawOutDevDirect( &rOutDev, &aPosAry );
        }
    }
}

// ------------------------------------------------------------------

void OutputDevice::CopyArea( const Point& rDestPt,
                             const Point& rSrcPt,  const Size& rSrcSize,
                             USHORT nFlags )
{
    DBG_TRACE( "OutputDevice::CopyArea()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_ASSERT( meOutDevType != OUTDEV_PRINTER, "Don't use OutputDevice::CopyArea(...) with printer devices!" );

    if ( meOutDevType == OUTDEV_PRINTER || ImplIsRecordLayout() )
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

                mpGraphics->CopyArea( aPosAry.mnDestX, aPosAry.mnDestY,
                                      aPosAry.mnSrcX, aPosAry.mnSrcY,
                                      aPosAry.mnSrcWidth, aPosAry.mnSrcHeight,
                                      SAL_COPYAREA_WINDOWINVALIDATE, this );
            }
            else
            {
                aPosAry.mnDestWidth  = aPosAry.mnSrcWidth;
                aPosAry.mnDestHeight = aPosAry.mnSrcHeight;
                mpGraphics->CopyBits( &aPosAry, NULL, this, NULL );
            }
        }
    }

    SetRasterOp( eOldRop );

    if( mpAlphaVDev )
        mpAlphaVDev->CopyArea( rDestPt, rSrcPt, rSrcSize, nFlags );
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

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    // ClipRegion zuruecksetzen
    if ( rRegion.IsNull() )
        mpGraphics->ResetClipRegion();
    else
        ImplSelectClipRegion( mpGraphics, rRegion, this );

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

    if( ImplIsRecordLayout() )
        return;

    const Size aSizePix( rBitmap.GetSizePixel() );
    ImplDrawBitmap( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmap, META_BMP_ACTION );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawRect( Rectangle(rDestPt, PixelToLogic( aSizePix )) );
}

// ------------------------------------------------------------------

void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap )
{
    DBG_TRACE( "OutputDevice::DrawBitmap( Size )" );

    if( ImplIsRecordLayout() )
        return;

    ImplDrawBitmap( rDestPt, rDestSize, Point(), rBitmap.GetSizePixel(), rBitmap, META_BMPSCALE_ACTION );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawRect( Rectangle(rDestPt, rDestSize) );
}

// ------------------------------------------------------------------

void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize,
                               const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                               const Bitmap& rBitmap )
{
    DBG_TRACE( "OutputDevice::DrawBitmap( Point, Size )" );

    if( ImplIsRecordLayout() )
        return;

    ImplDrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmap, META_BMPSCALEPART_ACTION );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawRect( Rectangle(rDestPt, rDestSize) );
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

    if( !aBmp.IsEmpty() )
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

            mpGraphics->DrawBitmap( &aPosAry, *aBmp.ImplGetImpBitmap()->ImplGetSalBitmap(), this );
        }
    }
}

// ------------------------------------------------------------------

void OutputDevice::DrawBitmapEx( const Point& rDestPt,
                                 const BitmapEx& rBitmapEx )
{
    DBG_TRACE( "OutputDevice::DrawBitmapEx()" );

    if( ImplIsRecordLayout() )
        return;

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

    if( ImplIsRecordLayout() )
        return;

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

    if( ImplIsRecordLayout() )
        return;

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
        if( rBitmapEx.IsAlpha() )
        {
            // #107169# For true alpha bitmaps, no longer masking the
            // bitmap, but perform a full alpha blend against a white
            // background here.
            Bitmap aBmp( aBmpEx.GetBitmap() );
            aBmp.Blend( aBmpEx.GetAlpha(), Color( COL_WHITE) );
            DrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, aBmp );
        }
        else
        {
            Bitmap aBmp( aBmpEx.GetBitmap() ), aMask( aBmpEx.GetMask() );
            aBmp.Replace( aMask, Color( COL_WHITE ) );
            ImplPrintTransparent( aBmp, aMask, rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );
        }
        return;
    }
    else if( rBitmapEx.IsAlpha() )
    {
        ImplDrawAlpha( aBmpEx.GetBitmap(), aBmpEx.GetAlpha(), rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );
        return;
    }

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

            if( nMirrFlags )
                aBmpEx.Mirror( nMirrFlags );

            const ImpBitmap* pImpBmp = aBmpEx.ImplGetBitmapImpBitmap();
            const ImpBitmap* pMaskBmp = aBmpEx.ImplGetMaskImpBitmap();

            if ( pMaskBmp )
            {
                // #4919452# reduce operation area to bounds of
                // cliprect. since masked transparency involves
                // creation of a large vdev and copying the screen
                // content into that (slooow read from framebuffer),
                // that should considerably increase performance for
                // large bitmaps and small clippings.

                // Note that this optimisation is a workaround for a
                // Writer peculiarity, namely, to decompose background
                // graphics into myriads of disjunct, tiny
                // rectangles. That otherwise kills us here, since for
                // transparent output, SAL always prepares the whole
                // bitmap, if aPosAry contains the whole bitmap (and
                // it's _not_ to blame for that).

                // Note the call to ImplPixelToDevicePixel(), since
                // aPosAry already contains the mnOutOff-offsets, they
                // also have to be applied to the region
                Rectangle aClipRegionBounds( ImplPixelToDevicePixel(maRegion).GetBoundRect() );

                // TODO: Also respect scaling (that's a bit tricky,
                // since the source points have to move fractional
                // amounts (which is not possible, thus has to be
                // emulated by increases copy area)
                // const double nScaleX( aPosAry.mnDestWidth / aPosAry.mnSrcWidth );
                // const double nScaleY( aPosAry.mnDestHeight / aPosAry.mnSrcHeight );

                // for now, only identity scales allowed
                if( !aClipRegionBounds.IsEmpty() &&
                    aPosAry.mnDestWidth == aPosAry.mnSrcWidth &&
                    aPosAry.mnDestHeight == aPosAry.mnSrcHeight )
                {
                    // now intersect dest rect with clip region
                    aClipRegionBounds.Intersection( Rectangle( aPosAry.mnDestX,
                                                               aPosAry.mnDestY,
                                                               aPosAry.mnDestX + aPosAry.mnDestWidth - 1,
                                                               aPosAry.mnDestY + aPosAry.mnDestHeight - 1 ) );

                    // Note: I could theoretically optimize away the
                    // DrawBitmap below, if the region is empty
                    // here. Unfortunately, cannot rule out that
                    // somebody relies on the side effects.
                    if( !aClipRegionBounds.IsEmpty() )
                    {
                        aPosAry.mnSrcX += aClipRegionBounds.Left() - aPosAry.mnDestX;
                        aPosAry.mnSrcY += aClipRegionBounds.Top() - aPosAry.mnDestY;
                        aPosAry.mnSrcWidth = aClipRegionBounds.GetWidth();
                        aPosAry.mnSrcHeight = aClipRegionBounds.GetHeight();

                        aPosAry.mnDestX = aClipRegionBounds.Left();
                        aPosAry.mnDestY = aClipRegionBounds.Top();
                        aPosAry.mnDestWidth = aClipRegionBounds.GetWidth();
                        aPosAry.mnDestHeight = aClipRegionBounds.GetHeight();
                    }
                }

                mpGraphics->DrawBitmap( &aPosAry, *pImpBmp->ImplGetSalBitmap(),
                                        *pMaskBmp->ImplGetSalBitmap(),
                                        this );

                // #110958# Paint mask to alpha channel. Luckily, the
                // black and white representation of the mask maps to
                // the alpha channel

                // #i25167# Restrict mask painting to _opaque_ areas
                // of the mask, otherwise we spoil areas where no
                // bitmap content was ever visible. Interestingly
                // enough, this can be achieved by taking the mask as
                // the transparency mask of itself
                if( mpAlphaVDev )
                    mpAlphaVDev->DrawBitmapEx( rDestPt,
                                               rDestSize,
                                               BitmapEx( rBitmapEx.GetMask(),
                                                         rBitmapEx.GetMask() ) );
            }
            else
            {
                mpGraphics->DrawBitmap( &aPosAry, *pImpBmp->ImplGetSalBitmap(), this );

                if( mpAlphaVDev )
                    mpAlphaVDev->DrawRect( Rectangle(rDestPt, rDestSize) );
            }
        }
    }
}

// ------------------------------------------------------------------

void OutputDevice::DrawMask( const Point& rDestPt,
                             const Bitmap& rBitmap, const Color& rMaskColor )
{
    DBG_TRACE( "OutputDevice::DrawMask()" );

    if( ImplIsRecordLayout() )
        return;

    const Size aSizePix( rBitmap.GetSizePixel() );
    ImplDrawMask( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmap, rMaskColor, META_MASK_ACTION );

    // TODO: Use mask here
    if( mpAlphaVDev )
        mpAlphaVDev->DrawRect( Rectangle(rDestPt, PixelToLogic( aSizePix )) );
}

// ------------------------------------------------------------------

void OutputDevice::DrawMask( const Point& rDestPt, const Size& rDestSize,
                             const Bitmap& rBitmap, const Color& rMaskColor )
{
    DBG_TRACE( "OutputDevice::DrawMask( Size )" );

    if( ImplIsRecordLayout() )
        return;

    ImplDrawMask( rDestPt, rDestSize, Point(), rBitmap.GetSizePixel(), rBitmap, rMaskColor, META_MASKSCALE_ACTION );

    // TODO: Use mask here
    if( mpAlphaVDev )
        mpAlphaVDev->DrawRect( Rectangle(rDestPt, rDestSize) );
}

// ------------------------------------------------------------------

void OutputDevice::DrawMask( const Point& rDestPt, const Size& rDestSize,
                             const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                             const Bitmap& rBitmap, const Color& rMaskColor )
{
    DBG_TRACE( "OutputDevice::DrawMask( Point, Size )" );

    if( ImplIsRecordLayout() )
        return;

    ImplDrawMask( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmap, rMaskColor, META_MASKSCALEPART_ACTION );

    // TODO: Use mask here
    if( mpAlphaVDev )
        mpAlphaVDev->DrawRect( Rectangle(rDestPt, rDestSize) );
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

    if ( OUTDEV_PRINTER == meOutDevType )
    {
        ImplPrintMask( rBitmap, rMaskColor, rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );
        return;
    }

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

            if( nMirrFlags )
            {
                Bitmap aTmp( rBitmap );
                aTmp.Mirror( nMirrFlags );
                mpGraphics->DrawMask( &aPosAry, *aTmp.ImplGetImpBitmap()->ImplGetSalBitmap(),
                                      ImplColorToSal( rMaskColor ) , this);
            }
            else
                mpGraphics->DrawMask( &aPosAry, *pImpBmp->ImplGetSalBitmap(),
                                      ImplColorToSal( rMaskColor ), this );

        }
    }
}

// ------------------------------------------------------------------

void OutputDevice::DrawImage( const Point& rPos, const Image& rImage, USHORT nStyle )
{
    DBG_ASSERT( GetOutDevType() != OUTDEV_PRINTER, "DrawImage(): Images can't be drawn on any mprinter" );

    if( !rImage.mpImplData || ImplIsRecordLayout() )
        return;

    switch( rImage.mpImplData->meType )
    {
        case IMAGETYPE_BITMAP:
            DrawBitmap( rPos, *static_cast< Bitmap* >( rImage.mpImplData->mpData ) );
        break;

        case IMAGETYPE_IMAGE:
        {
            ImplImageData* pData = static_cast< ImplImageData* >( rImage.mpImplData->mpData );

            if( !pData->mpImageBitmap )
            {
                const Size aSize( pData->maBmpEx.GetSizePixel() );

                pData->mpImageBitmap = new ImplImageBmp;
                pData->mpImageBitmap->Create( pData->maBmpEx, aSize.Width(), aSize.Height(), 1 );
            }

            pData->mpImageBitmap->Draw( 0, this, rPos, nStyle );
        }
        break;

        case IMAGETYPE_IMAGEREF:
        {
            ImplImageRefData* pData = static_cast< ImplImageRefData* >( rImage.mpImplData->mpData );
            pData->mpImplData->mpImageBitmap->Draw( pData->mnIndex, this, rPos, nStyle );
        }
        break;

        default:
        break;
    }
}

// ------------------------------------------------------------------

void OutputDevice::DrawImage( const Point& rPos, const Size& rSize,
                              const Image& rImage, USHORT nStyle )
{
    DBG_ASSERT( GetOutDevType() != OUTDEV_PRINTER, "DrawImage(): Images can't be drawn on any mprinter" );

    if( rImage.mpImplData && !ImplIsRecordLayout() )
    {
        switch( rImage.mpImplData->meType )
        {
            case IMAGETYPE_BITMAP:
                DrawBitmap( rPos, rSize, *static_cast< Bitmap* >( rImage.mpImplData->mpData ) );
            break;

            case IMAGETYPE_IMAGE:
            {
                ImplImageData* pData = static_cast< ImplImageData* >( rImage.mpImplData->mpData );

                if ( !pData->mpImageBitmap )
                {
                    const Size aSize( pData->maBmpEx.GetSizePixel() );

                    pData->mpImageBitmap = new ImplImageBmp;
                    pData->mpImageBitmap->Create( pData->maBmpEx, aSize.Width(), aSize.Height(), 1 );
                }

                pData->mpImageBitmap->Draw( 0, this, rPos, nStyle, &rSize );
            }
            break;

            case IMAGETYPE_IMAGEREF:
            {
                ImplImageRefData* pData = static_cast< ImplImageRefData* >( rImage.mpImplData->mpData );
                pData->mpImplData->mpImageBitmap->Draw( pData->mnIndex, this, rPos, nStyle, &rSize );
            }
            break;

            default:
            break;
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

    if ( mpGraphics || ( (OutputDevice*) this )->ImplGetGraphics() )
    {
        if ( nWidth && nHeight )
        {
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
                            (((OutputDevice*)&aVDev)->mpGraphics)->CopyBits( &aPosAry, mpGraphics, this, this );

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
                SalBitmap* pSalBmp = mpGraphics->GetBitmap( nX, nY, nWidth, nHeight, this );

                if( pSalBmp )
                {
                    ImpBitmap* pImpBmp = new ImpBitmap;
                    pImpBmp->ImplSetSalBitmap( pSalBmp );
                    aBmp.ImplSetImpBitmap( pImpBmp );
                }
            }
        }
    }

    return aBmp;
}

// ------------------------------------------------------------------

BitmapEx OutputDevice::GetBitmapEx( const Point& rSrcPt, const Size& rSize ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    // #110958# Extract alpha value from VDev, if any
    if( mpAlphaVDev )
    {
        Bitmap aAlphaBitmap( mpAlphaVDev->GetBitmap( rSrcPt, rSize ) );

        // ensure 8 bit alpha
        if( aAlphaBitmap.GetBitCount() > 8 )
            aAlphaBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );

        return BitmapEx(GetBitmap( rSrcPt, rSize ), AlphaMask( aAlphaBitmap ) );
    }
    else
        return GetBitmap( rSrcPt, rSize );
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

    if ( mpGraphics || ((OutputDevice*)this)->ImplGetGraphics() )
    {
        if ( mbInitClipRegion )
            ((OutputDevice*)this)->ImplInitClipRegion();

        if ( !mbOutputClipped )
        {
            const long      nX = ImplLogicXToDevicePixel( rPt.X() );
            const long      nY = ImplLogicYToDevicePixel( rPt.Y() );
            const SalColor  aSalCol = mpGraphics->GetPixel( nX, nY, this );
            aColor.SetRed( SALCOLOR_RED( aSalCol ) );
            aColor.SetGreen( SALCOLOR_GREEN( aSalCol ) );
            aColor.SetBlue( SALCOLOR_BLUE( aSalCol ) );
        }
    }
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
                                                                   ImplLogicYToDevicePixel( rPt.Y() ) , this) );

                    rCol.SetRed( SALCOLOR_RED( aSalCol ) );
                    rCol.SetGreen( SALCOLOR_GREEN( aSalCol ) );
                    rCol.SetBlue( SALCOLOR_BLUE( aSalCol ) );
                }
            }
        }
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

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ImplIsRecordLayout() )
        return;

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

    mpGraphics->DrawPixel( aPt.X(), aPt.Y(), this );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPixel( rPt );
}

// -----------------------------------------------------------------------

void OutputDevice::DrawPixel( const Point& rPt, const Color& rColor )
{
    DBG_TRACE( "OutputDevice::DrawPixel()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );

    if( mnDrawMode & ( DRAWMODE_BLACKLINE | DRAWMODE_WHITELINE |
                       DRAWMODE_GRAYLINE | DRAWMODE_GHOSTEDLINE |
                       DRAWMODE_SETTINGSLINE ) )
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
            else if( mnDrawMode & DRAWMODE_SETTINGSLINE )
            {
                aColor = GetSettings().GetStyleSettings().GetFontColor();
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

    if ( !IsDeviceOutputNecessary() || ImplIsColorTransparent( aColor ) || ImplIsRecordLayout() )
        return;

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

    mpGraphics->DrawPixel( aPt.X(), aPt.Y(), ImplColorToSal( aColor ), this );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPixel( rPt );
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

            if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
                return;

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
                    mpGraphics->DrawPixel( aPt.X(), aPt.Y(), ImplColorToSal( pColors[ i ] ), this );
                }
            }
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPixel( rPts, pColors );
}

// -----------------------------------------------------------------------

void OutputDevice::DrawPixel( const Polygon& rPts, const Color& rColor )
{
    if( rColor != COL_TRANSPARENT && ! ImplIsRecordLayout() )
    {
        const USHORT    nSize = rPts.GetSize();
        Color*          pColArray = new Color[ nSize ];

        for( USHORT i = 0; i < nSize; i++ )
            pColArray[ i ] = rColor;

        DrawPixel( rPts, pColArray );
        delete[] pColArray;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPixel( rPts, rColor );
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
        VirtualDevice* pOldVDev = mpAlphaVDev;

        Rectangle aBmpRect( aPt, rBmp.GetSizePixel() );

        if( !aBmpRect.Intersection( Rectangle( rSrcPtPixel, rSrcSizePixel ) ).IsEmpty() )
        {
            GDIMetaFile*    pOldMetaFile = mpMetaFile; mpMetaFile = NULL;
            const BOOL      bOldMap = mbMap; mbMap = FALSE;
            Bitmap          aBmp( GetBitmap( aDstRect.TopLeft(), aDstRect.GetSize() ) );

            // #109044# The generated bitmap need not necessarily be
            // of aDstRect dimensions, it's internally clipped to
            // window bounds. Thus, we correct the dest size here,
            // since we later use it (in nDstWidth/Height) for pixel
            // access)
            aDstRect.SetSize( aBmp.GetSizePixel() );

            // #110958# Disable alpha VDev, we're doing the necessary
            // stuff explicitely furher below
            if( mpAlphaVDev )
                mpAlphaVDev = NULL;

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

            BitmapReadAccess*   pP = ( (Bitmap&) rBmp ).AcquireReadAccess();
            BitmapReadAccess*   pA = ( (AlphaMask&) rAlpha ).AcquireReadAccess();

            DBG_ASSERT( pA->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL ||
                        pA->GetScanlineFormat() == BMP_FORMAT_8BIT_TC_MASK,
                        "OutputDevice::ImplDrawAlpha(): non-8bit alpha no longer supported!" );

            if( GetBitCount() <= 8 )
            {
                Bitmap              aDither( aBmp.GetSizePixel(), 8 );
                BitmapColor         aIndex( 0 );
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

                aBmp.ReleaseAccess( pB );
                aDither.ReleaseAccess( pW );
                DrawBitmap( aDstRect.TopLeft(), aDither );
            }
            else
            {
                BitmapWriteAccess*  pB = aBmp.AcquireWriteAccess();

                if( pP && pA && pB )
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

                aBmp.ReleaseAccess( pB );
                DrawBitmap( aDstRect.TopLeft(), aBmp );
            }

            // #110958# Enable alpha VDev again
            mpAlphaVDev = pOldVDev;

            // #110958# Perform merging of bitmap and VDev alpha channel
            if( mpAlphaVDev )
            {
                BOOL bOldMapMode( IsMapModeEnabled() );
                mpAlphaVDev->EnableMapMode(FALSE);
                Bitmap aBitmap( mpAlphaVDev->GetBitmap( aDstRect.TopLeft(), aDstRect.GetSize() ) );
                BitmapWriteAccess*  pW = aBitmap.AcquireWriteAccess();
                BYTE nAlpha;

                if( pW && pA )
                {
                    if( mpAlphaVDev->GetBitCount() < 8 )
                    {
                        // Less than 8 bit for backbuffer - try dithering of alpha channel
                        BitmapColor aIndex( 0 );

                        for( nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
                        {
                            const long nMapY = pMapY[ nY ];
                            const long nModY = ( nOutY & 0x0FL ) << 4L;
                            Scanline  pAScan = pA->GetScanline( nMapY );

                            for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                            {
                                const long  nMapX = pMapX[ nX ];
                                const ULONG nD = nVCLDitherLut[ nModY | ( nOutX & 0x0FL ) ];

                                nAlpha = pAScan[ nMapX ];

                                // Have to perform the compositing
                                // 'algebra' in the inverse alpha
                                // space (with 255 meaning opaque),
                                // otherwise, transitivity is not
                                // achieved.
                                nAlpha = 255-COLOR_CHANNEL_MERGE( 255, (BYTE) 255-pW->GetColor( nY, nX ), 255-nAlpha );

                                aIndex.SetIndex( (BYTE) ( nVCLRLut[ ( nVCLLut[ nAlpha ] + nD ) >> 16UL ] +
                                                          nVCLGLut[ ( nVCLLut[ nAlpha ] + nD ) >> 16UL ] +
                                                          nVCLBLut[ ( nVCLLut[ nAlpha ] + nD ) >> 16UL ] ) );
                                pW->SetPixel( nY, nX, aIndex );
                            }
                        }
                    }
                    else
                    {
                        if( pW->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL ||
                            pW->GetScanlineFormat() == BMP_FORMAT_8BIT_TC_MASK )
                        {
                            // optimized handling for 8 bit alpha - 8 bit backbuffer
                            for( nY = 0; nY < nDstHeight; nY++ )
                            {
                                const long  nMapY = pMapY[ nY ];
                                Scanline    pWScan = pW->GetScanline( nY );
                                Scanline    pAScan = pA->GetScanline( nMapY );

                                for( nX = 0; nX < nDstWidth; nX++ )
                                {
                                    const long nMapX = pMapX[ nX ];

                                    nAlpha = pAScan[ nMapX ];

                                    // Have to perform the compositing
                                    // 'algebra' in the inverse alpha
                                    // space (with 255 meaning opaque),
                                    // otherwise, transitivity is not
                                    // achieved.
                                    pWScan[ nX ] = 255 - COLOR_CHANNEL_MERGE( 255, 255-pWScan[ nX ], 255-nAlpha );
                                }
                            }
                        }
                        else
                        {
                            // optimized handling for 8 bit alpha - >8 bit backbuffer
                            for( nY = 0; nY < nDstHeight; nY++ )
                            {
                                const long  nMapY = pMapY[ nY ];
                                Scanline    pAScan = pA->GetScanline( nMapY );

                                for( nX = 0; nX < nDstWidth; nX++ )
                                {
                                    const long nMapX = pMapX[ nX ];
                                    nAlpha = pAScan[ nMapX ];

                                    // Have to perform the compositing
                                    // 'algebra' in the inverse alpha
                                    // space (with 255 meaning opaque),
                                    // otherwise, transitivity is not
                                    // achieved.
                                    nAlpha = 255 - COLOR_CHANNEL_MERGE( 255, 255-pW->GetColor( nY, nX ).GetBlue(), 255-nAlpha );

                                    pW->SetPixel( nY, nX, Color(nAlpha, nAlpha, nAlpha) );
                                }
                            }
                        }
                    }
                }

                aBitmap.ReleaseAccess( pW );
                 mpAlphaVDev->DrawBitmap( aDstRect.TopLeft(), aBitmap );
                mpAlphaVDev->EnableMapMode( bOldMapMode );
            }

            ( (Bitmap&) rBmp ).ReleaseAccess( pP );
            ( (AlphaMask&) rAlpha ).ReleaseAccess( pA );

            delete[] pMapX;
            delete[] pMapY;
            mbMap = bOldMap;
            mpMetaFile = pOldMetaFile;
        }
    }
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

    if( !rBmp.IsEmpty() && aSrcRect.GetWidth() && aSrcRect.GetHeight() && aDestSz.Width() && aDestSz.Height() )
    {
        Bitmap  aPaint( rBmp ), aMask( rMask );
        ULONG   nMirrFlags = 0UL;

        if( aMask.GetBitCount() > 1 )
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
            aMask.Crop( aSrcRect );
        }

        // destination mirrored
        if( nMirrFlags )
        {
            aPaint.Mirror( nMirrFlags );
            aMask.Mirror( nMirrFlags );
        }

        // we always want to have a mask
        if( aMask.IsEmpty() )
        {
            aMask = Bitmap( aSrcRect.GetSize(), 1 );
            aMask.Erase( Color( COL_BLACK ) );
        }

        // do painting
        const long      nSrcWidth = aSrcRect.GetWidth(), nSrcHeight = aSrcRect.GetHeight();
        long            nX, nY, nWorkX, nWorkY, nWorkWidth, nWorkHeight;
        long*           pMapX = new long[ nSrcWidth + 1 ];
        long*           pMapY = new long[ nSrcHeight + 1 ];
        const BOOL      bOldMap = mbMap;

        mbMap = FALSE;

        // create forward mapping tables
        for( nX = 0L; nX <= nSrcWidth; nX++ )
            pMapX[ nX ] = aDestPt.X() + FRound( (double) aDestSz.Width() * nX / nSrcWidth );

        for( nY = 0L; nY <= nSrcHeight; nY++ )
            pMapY[ nY ] = aDestPt.Y() + FRound( (double) aDestSz.Height() * nY / nSrcHeight );

        // walk through all rectangles of mask
        Region          aWorkRgn( aMask.CreateRegion( COL_BLACK, Rectangle( Point(), aMask.GetSizePixel() ) ) );
        ImplRegionInfo  aInfo;
        BOOL            bRgnRect = aWorkRgn.ImplGetFirstRect( aInfo, nWorkX, nWorkY, nWorkWidth, nWorkHeight );

        while( bRgnRect )
        {
            Bitmap          aBandBmp( aPaint );
            const Rectangle aBandRect( Point( nWorkX, nWorkY ), Size( nWorkWidth, nWorkHeight ) );
            const Point     aMapPt( pMapX[ nWorkX ], pMapY[ nWorkY ] );
            const Size      aMapSz( pMapX[ nWorkX + nWorkWidth ] - aMapPt.X(), pMapY[ nWorkY + nWorkHeight ] - aMapPt.Y() );

            aBandBmp.Crop( aBandRect );
            ImplDrawBitmap( aMapPt, aMapSz, Point(), aBandBmp.GetSizePixel(), aBandBmp, META_BMPSCALEPART_ACTION );
            bRgnRect = aWorkRgn.ImplGetNextRect( aInfo, nWorkX, nWorkY, nWorkWidth, nWorkHeight );
        }

        mbMap = bOldMap;

        delete[] pMapX;
        delete[] pMapY;
    }
}

// ------------------------------------------------------------------------

void OutputDevice::ImplPrintMask( const Bitmap& rMask, const Color& rMaskColor,
                                  const Point& rDestPt, const Size& rDestSize,
                                  const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    Point       aPt;
    Point       aDestPt( LogicToPixel( rDestPt ) );
    Size        aDestSz( LogicToPixel( rDestSize ) );
    Rectangle   aSrcRect( rSrcPtPixel, rSrcSizePixel );

    aSrcRect.Justify();

    if( !rMask.IsEmpty() && aSrcRect.GetWidth() && aSrcRect.GetHeight() && aDestSz.Width() && aDestSz.Height() )
    {
        Bitmap  aMask( rMask );
        ULONG   nMirrFlags = 0UL;

        if( aMask.GetBitCount() > 1 )
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
        if( aSrcRect != Rectangle( aPt, aMask.GetSizePixel() ) )
            aMask.Crop( aSrcRect );

        // destination mirrored
        if( nMirrFlags )
            aMask.Mirror( nMirrFlags );

        // do painting
        const long      nSrcWidth = aSrcRect.GetWidth(), nSrcHeight = aSrcRect.GetHeight();
        long            nX, nY, nWorkX, nWorkY, nWorkWidth, nWorkHeight;
        long*           pMapX = new long[ nSrcWidth + 1 ];
        long*           pMapY = new long[ nSrcHeight + 1 ];
        GDIMetaFile*    pOldMetaFile = mpMetaFile;
        const BOOL      bOldMap = mbMap;

        mpMetaFile = NULL;
        mbMap = FALSE;
        Push( PUSH_FILLCOLOR | PUSH_LINECOLOR );
        SetLineColor( rMaskColor );
        SetFillColor( rMaskColor );
        ImplInitLineColor();
        ImplInitFillColor();

        // create forward mapping tables
        for( nX = 0L; nX <= nSrcWidth; nX++ )
            pMapX[ nX ] = aDestPt.X() + FRound( (double) aDestSz.Width() * nX / nSrcWidth );

        for( nY = 0L; nY <= nSrcHeight; nY++ )
            pMapY[ nY ] = aDestPt.Y() + FRound( (double) aDestSz.Height() * nY / nSrcHeight );

        // walk through all rectangles of mask
        Region          aWorkRgn( aMask.CreateRegion( COL_BLACK, Rectangle( Point(), aMask.GetSizePixel() ) ) );
        ImplRegionInfo  aInfo;
        BOOL            bRgnRect = aWorkRgn.ImplGetFirstRect( aInfo, nWorkX, nWorkY, nWorkWidth, nWorkHeight );

        while( bRgnRect )
        {
            const Point aMapPt( pMapX[ nWorkX ], pMapY[ nWorkY ] );
            const Size  aMapSz( pMapX[ nWorkX + nWorkWidth ] - aMapPt.X(), pMapY[ nWorkY + nWorkHeight ] - aMapPt.Y() );

            DrawRect( Rectangle( aMapPt, aMapSz ) );
            bRgnRect = aWorkRgn.ImplGetNextRect( aInfo, nWorkX, nWorkY, nWorkWidth, nWorkHeight );
        }

        Pop();
        delete[] pMapX;
        delete[] pMapY;
        mbMap = bOldMap;
        mpMetaFile = pOldMetaFile;
    }
}
