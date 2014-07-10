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

#include <tools/debug.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/window.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/outdev.hxx>
#include <vcl/image.hxx>
#include <bmpfast.hxx>
#include <salbmp.hxx>
#include <salgdi.hxx>
#include <impbmp.hxx>
#include <sallayout.hxx>
#include <image.h>
#include <outdev.h>
#include <window.h>
#include <outdata.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

DBG_NAMEEX( OutputDevice )

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

extern const sal_uLong nVCLRLut[ 6 ];
extern const sal_uLong nVCLGLut[ 6 ];
extern const sal_uLong nVCLBLut[ 6 ];
extern const sal_uLong nVCLDitherLut[ 256 ];
extern const sal_uLong nVCLLut[ 256 ];

sal_uLong ImplAdjustTwoRect( SalTwoRect& rTwoRect, const Size& rSizePix )
{
    sal_uLong nMirrFlags = 0;

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

void ImplAdjustTwoRect( SalTwoRect& rTwoRect, const Rectangle& rValidSrcRect )
{
    if( ( rTwoRect.mnSrcX < rValidSrcRect.Left() ) || ( rTwoRect.mnSrcX >= rValidSrcRect.Right() ) ||
        ( rTwoRect.mnSrcY < rValidSrcRect.Top() ) || ( rTwoRect.mnSrcY >= rValidSrcRect.Bottom() ) ||
        ( ( rTwoRect.mnSrcX + rTwoRect.mnSrcWidth ) > rValidSrcRect.Right() ) ||
        ( ( rTwoRect.mnSrcY + rTwoRect.mnSrcHeight ) > rValidSrcRect.Bottom() ) )
    {
        const Rectangle aSourceRect( Point( rTwoRect.mnSrcX, rTwoRect.mnSrcY ),
                                     Size( rTwoRect.mnSrcWidth, rTwoRect.mnSrcHeight ) );
        Rectangle       aCropRect( aSourceRect );

        aCropRect.Intersection( rValidSrcRect );

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
}

void OutputDevice::ImplDrawOutDevDirect( const OutputDevice* pSrcDev, SalTwoRect& rPosAry )
{
    SalGraphics*        pGraphics2;

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
            if ( ((Window*)this)->mpWindowImpl->mpFrameWindow == ((Window*)pSrcDev)->mpWindowImpl->mpFrameWindow )
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
    const Rectangle aSrcOutRect( Point( pSrcDev->mnOutOffX, pSrcDev->mnOutOffY ),
                                 Size( pSrcDev->mnOutWidth, pSrcDev->mnOutHeight ) );

    ImplAdjustTwoRect( rPosAry, aSrcOutRect );

    if ( rPosAry.mnSrcWidth && rPosAry.mnSrcHeight && rPosAry.mnDestWidth && rPosAry.mnDestHeight )
    {
        // --- RTL --- if this is no window, but pSrcDev is a window
        // mirroring may be required
        // because only windows have a SalGraphicsLayout
        // mirroring is performed here
        if( (GetOutDevType() != OUTDEV_WINDOW) && pGraphics2 && (pGraphics2->GetLayout() & SAL_LAYOUT_BIDI_RTL) )
        {
            SalTwoRect aPosAry2 = rPosAry;
            pGraphics2->mirror( aPosAry2.mnSrcX, aPosAry2.mnSrcWidth, pSrcDev );
            mpGraphics->CopyBits( aPosAry2, pGraphics2, this, pSrcDev );
        }
        else
            mpGraphics->CopyBits( rPosAry, pGraphics2, this, pSrcDev );
    }
}

void OutputDevice::DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                               const Point& rSrcPt,  const Size& rSrcSize )
{
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

    SalTwoRect aPosAry;
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

        const Rectangle aSrcOutRect( Point( mnOutOffX, mnOutOffY ),
                                     Size( mnOutWidth, mnOutHeight ) );

        ImplAdjustTwoRect( aPosAry, aSrcOutRect );

        if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
            mpGraphics->CopyBits( aPosAry, NULL, this, NULL );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawOutDev( rDestPt, rDestSize, rSrcPt, rSrcSize );
}

void OutputDevice::DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                               const Point& rSrcPt,  const Size& rSrcSize,
                               const OutputDevice& rOutDev )
{
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

    SalTwoRect aPosAry;
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
            ImplDrawOutDevDirect( &rOutDev, aPosAry );

            // #i32109#: make destination rectangle opaque - source has no alpha
            mpAlphaVDev->ImplFillOpaqueRectangle( Rectangle(rDestPt, rDestSize) );
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
            ImplDrawOutDevDirect( &rOutDev, aPosAry );
        }
    }
}

void OutputDevice::CopyArea( const Point& rDestPt,
                             const Point& rSrcPt,  const Size& rSrcSize,
                             sal_uInt16 nFlags )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_ASSERT( meOutDevType != OUTDEV_PRINTER, "Don't use OutputDevice::CopyArea(...) with printer devices!" );

    if ( meOutDevType == OUTDEV_PRINTER || ImplIsRecordLayout() )
        return;

    RasterOp eOldRop = GetRasterOp();
    SetRasterOp( ROP_OVERPAINT );

    OUTDEV_INIT();

    SalTwoRect aPosAry;
    aPosAry.mnSrcWidth   = ImplLogicWidthToDevicePixel( rSrcSize.Width() );
    aPosAry.mnSrcHeight  = ImplLogicHeightToDevicePixel( rSrcSize.Height() );

    if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight )
    {
        aPosAry.mnSrcX       = ImplLogicXToDevicePixel( rSrcPt.X() );
        aPosAry.mnSrcY       = ImplLogicYToDevicePixel( rSrcPt.Y() );
        aPosAry.mnDestX      = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY      = ImplLogicYToDevicePixel( rDestPt.Y() );
        aPosAry.mnDestWidth  = aPosAry.mnSrcWidth;
        aPosAry.mnDestHeight = aPosAry.mnSrcHeight;

        const Rectangle aSrcOutRect( Point( mnOutOffX, mnOutOffY ),
                                     Size( mnOutWidth, mnOutHeight ) );
        const Rectangle aSrcRect( Point( aPosAry.mnSrcX, aPosAry.mnSrcY ),
                                  Size( aPosAry.mnSrcWidth, aPosAry.mnSrcHeight ) );

        ImplAdjustTwoRect( aPosAry, aSrcOutRect );

        if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
        {
            if ( (meOutDevType == OUTDEV_WINDOW) && (nFlags & COPYAREA_WINDOWINVALIDATE) )
            {
                ((Window*)this)->ImplMoveAllInvalidateRegions( aSrcRect,
                                                               aPosAry.mnDestX-aPosAry.mnSrcX,
                                                               aPosAry.mnDestY-aPosAry.mnSrcY,
                                                               false );

                mpGraphics->CopyArea( aPosAry.mnDestX, aPosAry.mnDestY,
                                      aPosAry.mnSrcX, aPosAry.mnSrcY,
                                      aPosAry.mnSrcWidth, aPosAry.mnSrcHeight,
                                      SAL_COPYAREA_WINDOWINVALIDATE, this );
            }
            else
            {
                aPosAry.mnDestWidth  = aPosAry.mnSrcWidth;
                aPosAry.mnDestHeight = aPosAry.mnSrcHeight;
                mpGraphics->CopyBits( aPosAry, NULL, this, NULL );
            }
        }
    }

    SetRasterOp( eOldRop );

    if( mpAlphaVDev )
        mpAlphaVDev->CopyArea( rDestPt, rSrcPt, rSrcSize, nFlags );
}

void OutputDevice::ImplDrawFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                     const OutputDevice& rOutDev, const Region& rRegion )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    GDIMetaFile*    pOldMetaFile = mpMetaFile;
    bool            bOldMap = mbMap;
    RasterOp        eOldROP = GetRasterOp();
    mpMetaFile = NULL;
    mbMap = false;
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
        ImplSelectClipRegion( rRegion );

    SalTwoRect aPosAry;
    aPosAry.mnSrcX       = rDevPt.X();
    aPosAry.mnSrcY       = rDevPt.Y();
    aPosAry.mnSrcWidth   = rDevSize.Width();
    aPosAry.mnSrcHeight  = rDevSize.Height();
    aPosAry.mnDestX      = rPt.X();
    aPosAry.mnDestY      = rPt.Y();
    aPosAry.mnDestWidth  = rDevSize.Width();
    aPosAry.mnDestHeight = rDevSize.Height();
    ImplDrawOutDevDirect( &rOutDev, aPosAry );

    // Ensure that ClipRegion is recalculated and set
    mbInitClipRegion = true;

    SetRasterOp( eOldROP );
    mbMap = bOldMap;
    mpMetaFile = pOldMetaFile;
}

void OutputDevice::ImplGetFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                    OutputDevice& rDev )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    bool bOldMap = mbMap;
    mbMap = false;
    rDev.DrawOutDev( rDevPt, rDevSize, rPt, rDevSize, *this );
    mbMap = bOldMap;
}

void OutputDevice::DrawBitmap( const Point& rDestPt, const Bitmap& rBitmap )
{
    if( ImplIsRecordLayout() )
        return;

    const Size aSizePix( rBitmap.GetSizePixel() );
    ImplDrawBitmap( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmap, META_BMP_ACTION );

    if( mpAlphaVDev )
    {
        // #i32109#: Make bitmap area opaque
        mpAlphaVDev->ImplFillOpaqueRectangle( Rectangle(rDestPt, PixelToLogic( aSizePix )) );
    }
}

void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap )
{
    if( ImplIsRecordLayout() )
        return;

    ImplDrawBitmap( rDestPt, rDestSize, Point(), rBitmap.GetSizePixel(), rBitmap, META_BMPSCALE_ACTION );

    if( mpAlphaVDev )
    {
        // #i32109#: Make bitmap area opaque
        mpAlphaVDev->ImplFillOpaqueRectangle( Rectangle(rDestPt, rDestSize) );
    }
}

void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize,
                               const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                               const Bitmap& rBitmap )
{
    if( ImplIsRecordLayout() )
        return;

    ImplDrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmap, META_BMPSCALEPART_ACTION );

    if( mpAlphaVDev )
    {
        // #i32109#: Make bitmap area opaque
        mpAlphaVDev->ImplFillOpaqueRectangle( Rectangle(rDestPt, rDestSize) );
    }
}

void OutputDevice::ImplDrawBitmap( const Point& rDestPt, const Size& rDestSize,
                                   const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                   const Bitmap& rBitmap, const sal_uLong nAction )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( ( mnDrawMode & DRAWMODE_NOBITMAP ) )
    {
        return;
    }
    if ( ROP_INVERT == meRasterOp )
    {
        DrawRect( Rectangle( rDestPt, rDestSize ) );
        return;
    }

    Bitmap aBmp( rBitmap );

    if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP |
                             DRAWMODE_GRAYBITMAP  | DRAWMODE_GHOSTEDBITMAP ) )
    {
        if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP ) )
        {
            sal_uInt8 cCmpVal;

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
        SalTwoRect aPosAry;

        aPosAry.mnSrcX = rSrcPtPixel.X();
        aPosAry.mnSrcY = rSrcPtPixel.Y();
        aPosAry.mnSrcWidth = rSrcSizePixel.Width();
        aPosAry.mnSrcHeight = rSrcSizePixel.Height();
        aPosAry.mnDestX = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY = ImplLogicYToDevicePixel( rDestPt.Y() );
        aPosAry.mnDestWidth = ImplLogicWidthToDevicePixel( rDestSize.Width() );
        aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

        if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
        {
            const sal_uLong nMirrFlags = ImplAdjustTwoRect( aPosAry, aBmp.GetSizePixel() );

            if ( nMirrFlags )
                aBmp.Mirror( nMirrFlags );

            if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
            {
                const double nScaleX = aPosAry.mnDestWidth  / static_cast<double>( aPosAry.mnSrcWidth );
                const double nScaleY = aPosAry.mnDestHeight / static_cast<double>( aPosAry.mnSrcHeight );
                // If subsampling, use Bitmap::Scale for subsampling for better quality.
                if ( meOutDevType != OUTDEV_PRINTER &&
                    nAction == META_BMPSCALE_ACTION &&
                    (nScaleX < 1.0 || nScaleY < 1.0) )
                {
                    aBmp.Scale ( nScaleX, nScaleY );
                    aPosAry.mnSrcWidth = aPosAry.mnDestWidth;
                    aPosAry.mnSrcHeight = aPosAry.mnDestHeight;
                }
                mpGraphics->DrawBitmap( aPosAry, *aBmp.ImplGetImpBitmap()->ImplGetSalBitmap(), this );
            }
        }
    }
}

void OutputDevice::DrawBitmapEx( const Point& rDestPt,
                                 const BitmapEx& rBitmapEx )
{
    if( ImplIsRecordLayout() )
        return;

    if( TRANSPARENT_NONE == rBitmapEx.GetTransparentType() )
    {
        DrawBitmap( rDestPt, rBitmapEx.GetBitmap() );
    }
    else
    {
        const Size aSizePix( rBitmapEx.GetSizePixel() );
        ImplDrawBitmapEx( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmapEx, META_BMPEX_ACTION );
    }
}

void OutputDevice::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                 const BitmapEx& rBitmapEx )
{
    if( ImplIsRecordLayout() )
        return;

    if ( TRANSPARENT_NONE == rBitmapEx.GetTransparentType() )
    {
        DrawBitmap( rDestPt, rDestSize, rBitmapEx.GetBitmap() );
    }
    else
    {
        ImplDrawBitmapEx( rDestPt, rDestSize, Point(), rBitmapEx.GetSizePixel(), rBitmapEx, META_BMPEXSCALE_ACTION );
    }
}

void OutputDevice::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                 const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                 const BitmapEx& rBitmapEx )
{
    if( ImplIsRecordLayout() )
        return;

    if( TRANSPARENT_NONE == rBitmapEx.GetTransparentType() )
    {
        DrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmapEx.GetBitmap() );
    }
    else
    {
        ImplDrawBitmapEx( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmapEx, META_BMPEXSCALEPART_ACTION );
    }
}

// ------------------------------------------------------------------

void OutputDevice::DrawTransformedBitmapEx(
    const basegfx::B2DHomMatrix& rTransformation,
    const BitmapEx& rBitmapEx)
{
    OSL_TRACE( "OutputDevice::DrawBitmapEx( Point, Size )" );

    if( ImplIsRecordLayout() )
        return;

    if(rBitmapEx.IsEmpty())
        return;

    if ( mnDrawMode & DRAWMODE_NOBITMAP )
        return;

    // decompose matrix to check rotation and shear
    basegfx::B2DVector aScale, aTranslate;
    double fRotate, fShearX;
    rTransformation.decompose(aScale, aTranslate, fRotate, fShearX);
    const bool bRotated(!basegfx::fTools::equalZero(fRotate));
    const bool bSheared(!basegfx::fTools::equalZero(fShearX));
    const bool bMirroredX(basegfx::fTools::less(aScale.getX(), 0.0));
    const bool bMirroredY(basegfx::fTools::less(aScale.getY(), 0.0));
    static bool bForceToOwnTransformer(false);

    if(!bForceToOwnTransformer && !bRotated && !bSheared && !bMirroredX && !bMirroredY)
    {
        // with no rotation, shear or mirroring it can be mapped to DrawBitmapEx
        // do *not* execute the mirroring here, it's done in the fallback
        const Point aDestPt(basegfx::fround(aTranslate.getX()), basegfx::fround(aTranslate.getY()));
        const Size aDestSize(basegfx::fround(aScale.getX()), basegfx::fround(aScale.getY()));

        DrawBitmapEx(aDestPt, aDestSize, rBitmapEx);
        return;
    }

    // we have rotation,shear or mirror, check if some crazy mode needs the
    // created transformed bitmap
    const bool bInvert(ROP_INVERT == meRasterOp);
    const bool bBitmapChangedColor(mnDrawMode & (DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP | DRAWMODE_GRAYBITMAP | DRAWMODE_GHOSTEDBITMAP));
    const bool bMetafile(mpMetaFile);
    const bool bPrinter(OUTDEV_PRINTER == meOutDevType);
    bool bDone(false);
    const basegfx::B2DHomMatrix aFullTransform(GetViewTransformation() * rTransformation);
    const bool bTryDirectPaint(!bInvert && !bBitmapChangedColor && !bMetafile && !bPrinter);

    if(!bForceToOwnTransformer && bTryDirectPaint)
    {
        // try to paint directly
        const basegfx::B2DPoint aNull(aFullTransform * basegfx::B2DPoint(0.0, 0.0));
        const basegfx::B2DPoint aTopX(aFullTransform * basegfx::B2DPoint(1.0, 0.0));
        const basegfx::B2DPoint aTopY(aFullTransform * basegfx::B2DPoint(0.0, 1.0));
        SalBitmap* pSalSrcBmp = rBitmapEx.GetBitmap().ImplGetImpBitmap()->ImplGetSalBitmap();
        SalBitmap* pSalAlphaBmp = 0;

        if(rBitmapEx.IsTransparent())
        {
            if(rBitmapEx.IsAlpha())
            {
                pSalAlphaBmp = rBitmapEx.GetAlpha().ImplGetImpBitmap()->ImplGetSalBitmap();
            }
            else
            {
                pSalAlphaBmp = rBitmapEx.GetMask().ImplGetImpBitmap()->ImplGetSalBitmap();
            }
        }

        bDone = mpGraphics->DrawTransformedBitmap(
            aNull,
            aTopX,
            aTopY,
            *pSalSrcBmp,
            pSalAlphaBmp,
            this);
    }

    if(!bDone)
    {
        // take the fallback when no rotate and shear, but mirror (else we would have done this above)
        if(!bForceToOwnTransformer && !bRotated && !bSheared)
        {
            // with no rotation or shear it can be mapped to DrawBitmapEx
            // do *not* execute the mirroring here, it's done in the fallback
            const Point aDestPt(basegfx::fround(aTranslate.getX()), basegfx::fround(aTranslate.getY()));
            const Size aDestSize(basegfx::fround(aScale.getX()), basegfx::fround(aScale.getY()));

            DrawBitmapEx(aDestPt, aDestSize, rBitmapEx);
            return;
        }

        // fallback; create transformed bitmap the hard way (back-transform
        // the pixels) and paint
        basegfx::B2DRange aVisibleRange(0.0, 0.0, 1.0, 1.0);

        // limit maximum area to something looking good for non-pixel-based targets (metafile, printer)
        // by using a fixed minimum (allow at least, but no need to utilize) for good smooting and an area
        // dependent of original size for good quality when e.g. rotated/sheared. Still, limit to a maximum
        // to avoid crashes/ressource problems (ca. 1500x3000 here)
        const Size& rOriginalSizePixel(rBitmapEx.GetSizePixel());
        const double fOrigArea(rOriginalSizePixel.Width() * rOriginalSizePixel.Height() * 0.5);
        const double fOrigAreaScaled(bSheared || bRotated ? fOrigArea * 1.44 : fOrigArea);
        double fMaximumArea(std::min(4500000.0, std::max(1000000.0, fOrigAreaScaled)));

        if(!bMetafile && !bPrinter)
        {
            // limit TargetRange to existing pixels (if pixel device)
            // first get discrete range of object
            basegfx::B2DRange aFullPixelRange(aVisibleRange);

            aFullPixelRange.transform(aFullTransform);

            if(basegfx::fTools::equalZero(aFullPixelRange.getWidth()) || basegfx::fTools::equalZero(aFullPixelRange.getHeight()))
            {
                // object is outside of visible area
                return;
            }

            // now get discrete target pixels; start with OutDev pixel size and evtl.
            // intersect with active clipping area
            basegfx::B2DRange aOutPixel(
                0.0,
                0.0,
                GetOutputSizePixel().Width(),
                GetOutputSizePixel().Height());

            if(IsClipRegion())
            {
                const Rectangle aRegionRectangle(GetActiveClipRegion().GetBoundRect());

                aOutPixel.intersect( // caution! Range from rectangle, one too much (!)
                    basegfx::B2DRange(
                        aRegionRectangle.Left(),
                        aRegionRectangle.Top(),
                        aRegionRectangle.Right() + 1,
                        aRegionRectangle.Bottom() + 1));
            }

            if(aOutPixel.isEmpty())
            {
                // no active output area
                return;
            }

            // if aFullPixelRange is not completely inside of aOutPixel,
            // reduction of target pixels is possible
            basegfx::B2DRange aVisiblePixelRange(aFullPixelRange);

            if(!aOutPixel.isInside(aFullPixelRange))
            {
                aVisiblePixelRange.intersect(aOutPixel);

                if(aVisiblePixelRange.isEmpty())
                {
                    // nothing in visible part, reduces to nothing
                    return;
                }

                // aVisiblePixelRange contains the reduced output area in
                // discrete coordinates. To make it useful everywhere, make it relative to
                // the object range
                basegfx::B2DHomMatrix aMakeVisibleRangeRelative;

                aVisibleRange = aVisiblePixelRange;
                aMakeVisibleRangeRelative.translate(
                    -aFullPixelRange.getMinX(),
                    -aFullPixelRange.getMinY());
                aMakeVisibleRangeRelative.scale(
                    1.0 / aFullPixelRange.getWidth(),
                    1.0 / aFullPixelRange.getHeight());
                aVisibleRange.transform(aMakeVisibleRangeRelative);
            }

            // for pixel devices, do *not* limit size, else OutputDevice::ImplDrawAlpha
            // will create another, badly scaled bitmap to do the job. Nonetheless, do a
            // maximum clipping of something big (1600x1280x2). Add 1.0 to avoid rounding
            // errors in rough estimations
            const double fNewMaxArea(aVisiblePixelRange.getWidth() * aVisiblePixelRange.getHeight());

            fMaximumArea = std::min(4096000.0, fNewMaxArea + 1.0);
        }

        if(!aVisibleRange.isEmpty())
        {
            static bool bDoSmoothAtAll(true);
            BitmapEx aTransformed(rBitmapEx);

            // #122923# when the result needs an alpha channel due to being rotated or sheared
            // and thus uncovering areas, add these channels so that the own transformer (used
            // in getTransformed) also creates a transformed alpha channel
            if(!aTransformed.IsTransparent() && (bSheared || bRotated))
            {
                // parts will be uncovered, extend aTransformed with a mask bitmap
                const Bitmap aContent(aTransformed.GetBitmap());
#if defined(MACOSX)
                AlphaMask aMaskBmp(aContent.GetSizePixel());
                aMaskBmp.Erase(0);
#else
                Bitmap aMaskBmp(aContent.GetSizePixel(), 1);
                aMaskBmp.Erase(Color(COL_BLACK)); // #122758# Initialize to non-transparent
#endif
                aTransformed = BitmapEx(aContent, aMaskBmp);
            }

            aTransformed = aTransformed.getTransformed(
                aFullTransform,
                aVisibleRange,
                fMaximumArea,
                bDoSmoothAtAll);
            basegfx::B2DRange aTargetRange(0.0, 0.0, 1.0, 1.0);

            // get logic object target range
            aTargetRange.transform(rTransformation);

            // get from unified/relative VisibleRange to logoc one
            aVisibleRange.transform(
                basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aTargetRange.getRange(),
                    aTargetRange.getMinimum()));

            // extract point and size; do not remove size, the bitmap may have been prepared reduced by purpose
            const Point aDestPt(basegfx::fround(aVisibleRange.getMinX()), basegfx::fround(aVisibleRange.getMinY()));
            const Size aDestSize(basegfx::fround(aVisibleRange.getWidth()), basegfx::fround(aVisibleRange.getHeight()));

            DrawBitmapEx(aDestPt, aDestSize, aTransformed);
        }
    }
}

void OutputDevice::ImplDrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                     const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                     const BitmapEx& rBitmapEx, const sal_uLong nAction )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    OSL_ENSURE(TRANSPARENT_NONE != rBitmapEx.GetTransparentType(), "ImplDrawBitmapEx not needed, no transparency in BitmapEx (!)");

    if ( mnDrawMode & DRAWMODE_NOBITMAP )
        return;

    if ( ROP_INVERT == meRasterOp )
    {
        DrawRect( Rectangle( rDestPt, rDestSize ) );
        return;
    }

    BitmapEx aBmpEx( rBitmapEx );

    if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP |
                             DRAWMODE_GRAYBITMAP | DRAWMODE_GHOSTEDBITMAP ) )
    {
        if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP ) )
        {
            Bitmap  aColorBmp( aBmpEx.GetSizePixel(), ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP ) ? 4 : 1 );
            sal_uInt8   cCmpVal;

            if ( mnDrawMode & DRAWMODE_BLACKBITMAP )
                cCmpVal = ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP ) ? 0x80 : 0;
            else
                cCmpVal = 255;

            aColorBmp.Erase( Color( cCmpVal, cCmpVal, cCmpVal ) );

            if( aBmpEx.IsAlpha() )
            {
                // Create one-bit mask out of alpha channel, by
                // thresholding it at alpha=0.5. As
                // DRAWMODE_BLACK/WHITEBITMAP requires monochrome
                // output, having alpha-induced grey levels is not
                // acceptable.
                Bitmap aMask( aBmpEx.GetAlpha().GetBitmap() );
                aMask.MakeMono( 129 );
                aBmpEx = BitmapEx( aColorBmp, aMask );
            }
            else
            {
                aBmpEx = BitmapEx( aColorBmp, aBmpEx.GetMask() );
            }
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
        if( aBmpEx.IsAlpha() )
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

    if(aBmpEx.IsAlpha())
    {
        ImplDrawAlpha( aBmpEx.GetBitmap(), aBmpEx.GetAlpha(), rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );
        return;
    }

    if( !( !aBmpEx ) )
    {
        SalTwoRect aPosAry;

        aPosAry.mnSrcX = rSrcPtPixel.X();
        aPosAry.mnSrcY = rSrcPtPixel.Y();
        aPosAry.mnSrcWidth = rSrcSizePixel.Width();
        aPosAry.mnSrcHeight = rSrcSizePixel.Height();
        aPosAry.mnDestX = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY = ImplLogicYToDevicePixel( rDestPt.Y() );
        aPosAry.mnDestWidth = ImplLogicWidthToDevicePixel( rDestSize.Width() );
        aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

        const sal_uLong nMirrFlags = ImplAdjustTwoRect( aPosAry, aBmpEx.GetSizePixel() );

        if( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
        {

            if( nMirrFlags )
                aBmpEx.Mirror( nMirrFlags );

            const SalBitmap* pSalSrcBmp = aBmpEx.ImplGetBitmapImpBitmap()->ImplGetSalBitmap();
            const ImpBitmap* pMaskBmp = aBmpEx.ImplGetMaskImpBitmap();

            if ( pMaskBmp )
            {
                SalBitmap* pSalAlphaBmp = pMaskBmp->ImplGetSalBitmap();
                bool bTryDirectPaint(pSalSrcBmp && pSalAlphaBmp);

                if(bTryDirectPaint)
                {
                    // only paint direct when no scaling and no MapMode, else the
                    // more expensive conversions may be done for short-time Bitmap/BitmapEx
                    // used for buffering only
                    if(!IsMapMode() && aPosAry.mnSrcWidth == aPosAry.mnDestWidth && aPosAry.mnSrcHeight == aPosAry.mnDestHeight)
                    {
                        bTryDirectPaint = false;
                    }
                }

                if(bTryDirectPaint && mpGraphics->DrawAlphaBitmap(aPosAry, *pSalSrcBmp, *pSalAlphaBmp, this))
                {
                    // tried to paint as alpha directly. If tis worked, we are done (except
                    // alpha, see below)
                }
                else
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

                    mpGraphics->DrawBitmap( aPosAry, *pSalSrcBmp,
                                            *pMaskBmp->ImplGetSalBitmap(),
                                            this );
                }

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
                                               BitmapEx( aBmpEx.GetMask(),
                                                         aBmpEx.GetMask() ) );
            }
            else
            {
                mpGraphics->DrawBitmap( aPosAry, *pSalSrcBmp, this );

                if( mpAlphaVDev )
                {
                    // #i32109#: Make bitmap area opaque
                    mpAlphaVDev->ImplFillOpaqueRectangle( Rectangle(rDestPt, rDestSize) );
                }
            }
        }
    }
}

void OutputDevice::DrawMask( const Point& rDestPt,
                             const Bitmap& rBitmap, const Color& rMaskColor )
{
    if( ImplIsRecordLayout() )
        return;

    const Size aSizePix( rBitmap.GetSizePixel() );
    ImplDrawMask( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmap, rMaskColor, META_MASK_ACTION );

    if( mpAlphaVDev )
    {
        const Bitmap& rMask( rBitmap.CreateMask( rMaskColor ) );

        // #i25167# Restrict mask painting to _opaque_ areas
        // of the mask, otherwise we spoil areas where no
        // bitmap content was ever visible. Interestingly
        // enough, this can be achieved by taking the mask as
        // the transparency mask of itself
        mpAlphaVDev->DrawBitmapEx( rDestPt,
                                   PixelToLogic( aSizePix ),
                                   BitmapEx( rMask, rMask ) );
    }
}

void OutputDevice::DrawMask( const Point& rDestPt, const Size& rDestSize,
                             const Bitmap& rBitmap, const Color& rMaskColor )
{
    if( ImplIsRecordLayout() )
        return;

    ImplDrawMask( rDestPt, rDestSize, Point(), rBitmap.GetSizePixel(), rBitmap, rMaskColor, META_MASKSCALE_ACTION );

    // TODO: Use mask here
    if( mpAlphaVDev )
    {
        const Bitmap& rMask( rBitmap.CreateMask( rMaskColor ) );

        // #i25167# Restrict mask painting to _opaque_ areas
        // of the mask, otherwise we spoil areas where no
        // bitmap content was ever visible. Interestingly
        // enough, this can be achieved by taking the mask as
        // the transparency mask of itself
        mpAlphaVDev->DrawBitmapEx( rDestPt,
                                   rDestSize,
                                   BitmapEx( rMask, rMask ) );
    }
}

void OutputDevice::DrawMask( const Point& rDestPt, const Size& rDestSize,
                             const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                             const Bitmap& rBitmap, const Color& rMaskColor )
{
    if( ImplIsRecordLayout() )
        return;

    ImplDrawMask( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmap, rMaskColor, META_MASKSCALEPART_ACTION );

    // TODO: Use mask here
    if( mpAlphaVDev )
    {
        const Bitmap& rMask( rBitmap.CreateMask( rMaskColor ) );

        // #i25167# Restrict mask painting to _opaque_ areas
        // of the mask, otherwise we spoil areas where no
        // bitmap content was ever visible. Interestingly
        // enough, this can be achieved by taking the mask as
        // the transparency mask of itself
        mpAlphaVDev->DrawBitmapEx( rDestPt,
                                   rDestSize,
                                   rSrcPtPixel,
                                   rSrcSizePixel,
                                   BitmapEx( rMask, rMask ) );
    }
}

void OutputDevice::ImplDrawMask( const Point& rDestPt, const Size& rDestSize,
                                 const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                 const Bitmap& rBitmap, const Color& rMaskColor,
                                 const sal_uLong nAction )
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
        SalTwoRect aPosAry;

        aPosAry.mnSrcX = rSrcPtPixel.X();
        aPosAry.mnSrcY = rSrcPtPixel.Y();
        aPosAry.mnSrcWidth = rSrcSizePixel.Width();
        aPosAry.mnSrcHeight = rSrcSizePixel.Height();
        aPosAry.mnDestX = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY = ImplLogicYToDevicePixel( rDestPt.Y() );
        aPosAry.mnDestWidth = ImplLogicWidthToDevicePixel( rDestSize.Width() );
        aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

        // we don't want to mirror via cooridates
        const sal_uLong nMirrFlags = ImplAdjustTwoRect( aPosAry, pImpBmp->ImplGetSize() );

        // check if output is necessary
        if( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
        {

            if( nMirrFlags )
            {
                Bitmap aTmp( rBitmap );
                aTmp.Mirror( nMirrFlags );
                mpGraphics->DrawMask( aPosAry, *aTmp.ImplGetImpBitmap()->ImplGetSalBitmap(),
                                      ImplColorToSal( rMaskColor ) , this);
            }
            else
                mpGraphics->DrawMask( aPosAry, *pImpBmp->ImplGetSalBitmap(),
                                      ImplColorToSal( rMaskColor ), this );

        }
    }
}

namespace
{
    BitmapEx makeDisabledBitmap(const Bitmap &rBitmap)
    {
        const Size aTotalSize( rBitmap.GetSizePixel() );
        Bitmap aGrey( aTotalSize, 8, &Bitmap::GetGreyPalette( 256 ) );
        AlphaMask aGreyAlphaMask( aTotalSize );
        BitmapReadAccess*  pBmp = const_cast<Bitmap&>(rBitmap).AcquireReadAccess();
        BitmapWriteAccess* pGrey = aGrey.AcquireWriteAccess();
        BitmapWriteAccess* pGreyAlphaMask = aGreyAlphaMask.AcquireWriteAccess();

        if( pBmp && pGrey && pGreyAlphaMask )
        {
            BitmapColor aGreyVal( 0 );
            BitmapColor aGreyAlphaMaskVal( 0 );
            const int nLeft = 0, nRight = aTotalSize.Width();
            const int nTop = 0, nBottom = nTop + aTotalSize.Height();

            for( int nY = nTop; nY < nBottom; ++nY )
            {
                for( int nX = nLeft; nX < nRight; ++nX )
                {
                    aGreyVal.SetIndex( pBmp->GetLuminance( nY, nX ) );
                    pGrey->SetPixel( nY, nX, aGreyVal );

                    aGreyAlphaMaskVal.SetIndex( static_cast< sal_uInt8 >( 128ul ) );
                    pGreyAlphaMask->SetPixel( nY, nX, aGreyAlphaMaskVal );
                }
            }
        }

        const_cast<Bitmap&>(rBitmap).ReleaseAccess( pBmp );
        aGrey.ReleaseAccess( pGrey );
        aGreyAlphaMask.ReleaseAccess( pGreyAlphaMask );
        return BitmapEx( aGrey, aGreyAlphaMask );
    }
}

void OutputDevice::DrawImage( const Point& rPos, const Image& rImage, sal_uInt16 nStyle )
{
    DBG_ASSERT( GetOutDevType() != OUTDEV_PRINTER, "DrawImage(): Images can't be drawn on any mprinter" );

    if( !rImage.mpImplData || ImplIsRecordLayout() )
        return;

    switch( rImage.mpImplData->meType )
    {
        case IMAGETYPE_BITMAP:
        {
            const Bitmap &rBitmap = *static_cast< Bitmap* >( rImage.mpImplData->mpData );
            if( nStyle & IMAGE_DRAW_DISABLE )
                DrawBitmapEx( rPos, makeDisabledBitmap(rBitmap) );
            else
                DrawBitmap( rPos, rBitmap );
        }
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

        default:
        break;
    }
}

void OutputDevice::DrawImage( const Point& rPos, const Size& rSize,
                              const Image& rImage, sal_uInt16 nStyle )
{
    DBG_ASSERT( GetOutDevType() != OUTDEV_PRINTER, "DrawImage(): Images can't be drawn on any mprinter" );

    if( rImage.mpImplData && !ImplIsRecordLayout() )
    {
        switch( rImage.mpImplData->meType )
        {
            case IMAGETYPE_BITMAP:
            {
                const Bitmap &rBitmap = *static_cast< Bitmap* >( rImage.mpImplData->mpData );
                if( nStyle & IMAGE_DRAW_DISABLE )
                    DrawBitmapEx( rPos, rSize, makeDisabledBitmap(rBitmap) );
                else
                    DrawBitmap( rPos, rSize, rBitmap );
            }
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

            default:
            break;
        }
    }
}

Bitmap OutputDevice::GetBitmap( const Point& rSrcPt, const Size& rSize ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    OSL_ENSURE(OUTDEV_PRINTER != GetOutDevType(), "OutputDevice::GetBitmap with sorce type OUTDEV_PRINTER should not be used (!)");

    Bitmap  aBmp;
    long    nX = ImplLogicXToDevicePixel( rSrcPt.X() );
    long    nY = ImplLogicYToDevicePixel( rSrcPt.Y() );
    long    nWidth = ImplLogicWidthToDevicePixel( rSize.Width() );
    long    nHeight = ImplLogicHeightToDevicePixel( rSize.Height() );

    if ( mpGraphics || ( (OutputDevice*) this )->ImplGetGraphics() )
    {
        if ( nWidth > 0 && nHeight  > 0 && nX <= (mnOutWidth + mnOutOffX) && nY <= (mnOutHeight + mnOutOffY))
        {
            Rectangle   aRect( Point( nX, nY ), Size( nWidth, nHeight ) );
            bool        bClipped = false;

            // X-Coordinate outside of draw area?
            if ( nX < mnOutOffX )
            {
                nWidth -= ( mnOutOffX - nX );
                nX = mnOutOffX;
                bClipped = true;
            }

            // Y-Coordinate outside of draw area?
            if ( nY < mnOutOffY )
            {
                nHeight -= ( mnOutOffY - nY );
                nY = mnOutOffY;
                bClipped = true;
            }

            // Width outside of draw area?
            if ( (nWidth + nX) > (mnOutWidth + mnOutOffX) )
            {
                nWidth  = mnOutOffX + mnOutWidth - nX;
                bClipped = true;
            }

            // Height outside of draw area?
            if ( (nHeight + nY) > (mnOutHeight + mnOutOffY) )
            {
                nHeight = mnOutOffY + mnOutHeight - nY;
                bClipped = true;
            }

            if ( bClipped )
            {
                // If the visible part has been clipped, we have to create a
                // Bitmap with the correct size in which we copy the clipped
                // Bitmap to the correct position.
                VirtualDevice aVDev( *this );

                if ( aVDev.SetOutputSizePixel( aRect.GetSize() ) )
                {
                    if ( ((OutputDevice*)&aVDev)->mpGraphics || ((OutputDevice*)&aVDev)->ImplGetGraphics() )
                    {
                        SalTwoRect aPosAry;

                        aPosAry.mnSrcX = nX;
                        aPosAry.mnSrcY = nY;
                        aPosAry.mnSrcWidth = nWidth;
                        aPosAry.mnSrcHeight = nHeight;
                        aPosAry.mnDestX = ( aRect.Left() < mnOutOffX ) ? ( mnOutOffX - aRect.Left() ) : 0L;
                        aPosAry.mnDestY = ( aRect.Top() < mnOutOffY ) ? ( mnOutOffY - aRect.Top() ) : 0L;
                        aPosAry.mnDestWidth = nWidth;
                        aPosAry.mnDestHeight = nHeight;

                        if ( (nWidth > 0) && (nHeight > 0) )
                        {
                            (((OutputDevice*)&aVDev)->mpGraphics)->CopyBits( aPosAry, mpGraphics, this, this );
                        }
                        else
                        {
                            OSL_ENSURE(false, "CopyBits with negative width or height (!)");
                        }

                        aBmp = aVDev.GetBitmap( Point(), aVDev.GetOutputSizePixel() );
                     }
                     else
                        bClipped = false;
                }
                else
                    bClipped = false;
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

void OutputDevice::DrawPixel( const Point& rPt )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPointAction( rPt ) );

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ImplIsRecordLayout() )
        return;

    Point aPt = ImplLogicToDevicePixel( rPt );

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

void OutputDevice::DrawPixel( const Point& rPt, const Color& rColor )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor = ImplDrawModeToColor( rColor );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPixelAction( rPt, aColor ) );

    if ( !IsDeviceOutputNecessary() || ImplIsColorTransparent( aColor ) || ImplIsRecordLayout() )
        return;

    Point aPt = ImplLogicToDevicePixel( rPt );

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

void OutputDevice::DrawPixel( const Polygon& rPts, const Color* pColors )
{
    if ( !pColors )
        DrawPixel( rPts, GetLineColor() );
    else
    {
        DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
        DBG_ASSERT( pColors, "OutputDevice::DrawPixel: No color array specified" );

        const sal_uInt16 nSize = rPts.GetSize();

        if ( nSize )
        {
            if ( mpMetaFile )
                for ( sal_uInt16 i = 0; i < nSize; i++ )
                    mpMetaFile->AddAction( new MetaPixelAction( rPts[ i ], pColors[ i ] ) );

            if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
                return;

            if ( mpGraphics || ImplGetGraphics() )
            {
                if ( mbInitClipRegion )
                    ImplInitClipRegion();

                if ( mbOutputClipped )
                    return;

                for ( sal_uInt16 i = 0; i < nSize; i++ )
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

void OutputDevice::DrawPixel( const Polygon& rPts, const Color& rColor )
{
    if( rColor != COL_TRANSPARENT && ! ImplIsRecordLayout() )
    {
        const sal_uInt16    nSize = rPts.GetSize();
        Color*          pColArray = new Color[ nSize ];

        for( sal_uInt16 i = 0; i < nSize; i++ )
            pColArray[ i ] = rColor;

        DrawPixel( rPts, pColArray );
        delete[] pColArray;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPixel( rPts, rColor );
}

namespace
{
    // Co = Cs + Cd*(1-As) premultiplied alpha -or-
    // Co = (AsCs + AdCd*(1-As)) / Ao
    inline sal_uInt8 lcl_calcColor( const sal_uInt8 nSourceColor, const sal_uInt8 nSourceAlpha,
                                    const sal_uInt8 nDstAlpha, const sal_uInt8 nResAlpha, const sal_uInt8 nDestColor )
    {
        int c = nResAlpha ? ( (int)nSourceAlpha*nSourceColor + (int)nDstAlpha*nDestColor -
                              (int)nDstAlpha*nDestColor*nSourceAlpha/255 ) / (int)nResAlpha : 0;
        return sal_uInt8( c );
    }

inline BitmapColor lcl_AlphaBlend( int nX,               int nY,
                                   const long            nMapX,
                                   const long            nMapY,
                                   BitmapReadAccess*     pP,
                                   BitmapReadAccess*     pA,
                                   BitmapReadAccess*     pB,
                                   BitmapWriteAccess*    pAlphaW,
                                   sal_uInt8&            nResAlpha )
{
    BitmapColor aDstCol,aSrcCol;
    aSrcCol = pP->GetColor( nMapY, nMapX );
    aDstCol = pB->GetColor( nY, nX );

    // vcl stores transparency, not alpha - invert it
    const sal_uInt8 nSrcAlpha = 255 - pA->GetPixelIndex( nMapY, nMapX );
    const sal_uInt8 nDstAlpha = 255 - pAlphaW->GetPixelIndex( nY, nX );

    // Perform porter-duff compositing 'over' operation
    //
    // Co = Cs + Cd*(1-As)
    // Ad = As + Ad*(1-As)
    nResAlpha = (int)nSrcAlpha + (int)nDstAlpha - (int)nDstAlpha*nSrcAlpha/255;

    aDstCol.SetRed( lcl_calcColor( aSrcCol.GetRed(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetRed() ) );
    aDstCol.SetBlue( lcl_calcColor( aSrcCol.GetBlue(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetBlue() ) );
    aDstCol.SetGreen( lcl_calcColor( aSrcCol.GetGreen(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetGreen() ) );

    return aDstCol;
}
}

Bitmap OutputDevice::ImplBlendWithAlpha( Bitmap              aBmp,
                                         BitmapReadAccess*   pP,
                                         BitmapReadAccess*   pA,
                                         const Rectangle&    aDstRect,
                                         const sal_Int32     nOffY,
                                         const sal_Int32     nDstHeight,
                                         const sal_Int32     nOffX,
                                         const sal_Int32     nDstWidth,
                                         const long*         pMapX,
                                         const long*         pMapY )
{
    BitmapColor aDstCol;
    Bitmap      res;
    int         nX, nY;
    sal_uInt8   nResAlpha;

    OSL_ENSURE(mpAlphaVDev,
               "ImplBlendWithAlpha(): call me only with valid alpha VDev!" );

    bool bOldMapMode( mpAlphaVDev->IsMapModeEnabled() );
    mpAlphaVDev->EnableMapMode(sal_False);

    Bitmap aAlphaBitmap( mpAlphaVDev->GetBitmap( aDstRect.TopLeft(), aDstRect.GetSize() ) );
    BitmapWriteAccess*  pAlphaW = aAlphaBitmap.AcquireWriteAccess();

    if( GetBitCount() <= 8 )
    {
        Bitmap              aDither( aBmp.GetSizePixel(), 8 );
        BitmapColor         aIndex( 0 );
        BitmapReadAccess*   pB = aBmp.AcquireReadAccess();
        BitmapWriteAccess*  pW = aDither.AcquireWriteAccess();

        if( pB && pP && pA && pW && pAlphaW )
        {
            int nOutY;

            for( nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
            {
                const long nMapY = pMapY[ nY ];
                const long nModY = ( nOutY & 0x0FL ) << 4L;
                int nOutX;

                for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                {
                    const long  nMapX = pMapX[ nX ];
                    const sal_uLong nD = nVCLDitherLut[ nModY | ( nOutX & 0x0FL ) ];

                    aDstCol = lcl_AlphaBlend( nX, nY, nMapX, nMapY, pP, pA, pB, pAlphaW, nResAlpha );

                    aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ ( nVCLLut[ aDstCol.GetRed() ] + nD ) >> 16UL ] +
                                              nVCLGLut[ ( nVCLLut[ aDstCol.GetGreen() ] + nD ) >> 16UL ] +
                                              nVCLBLut[ ( nVCLLut[ aDstCol.GetBlue() ] + nD ) >> 16UL ] ) );
                    pW->SetPixel( nY, nX, aIndex );

                    aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16UL ] +
                                                   nVCLGLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16UL ] +
                                                   nVCLBLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16UL ] ) );
                    pAlphaW->SetPixel( nY, nX, aIndex );
                }
            }
        }

        aBmp.ReleaseAccess( pB );
        aDither.ReleaseAccess( pW );
        res = aDither;
    }
    else
    {
        BitmapWriteAccess*  pB = aBmp.AcquireWriteAccess();
        if( pP && pA && pB )
        {
            for( nY = 0; nY < nDstHeight; nY++ )
            {
                const long  nMapY = pMapY[ nY ];

                for( nX = 0; nX < nDstWidth; nX++ )
                {
                    const long nMapX = pMapX[ nX ];
                    aDstCol = lcl_AlphaBlend( nX, nY, nMapX, nMapY, pP, pA, pB, pAlphaW, nResAlpha );

                    pB->SetPixel( nY, nX, aDstCol );
                    pAlphaW->SetPixel( nY, nX, Color(255L-nResAlpha, 255L-nResAlpha, 255L-nResAlpha) );
                }
            }
        }

        aBmp.ReleaseAccess( pB );
        res = aBmp;
    }

    aAlphaBitmap.ReleaseAccess( pAlphaW );
    mpAlphaVDev->DrawBitmap( aDstRect.TopLeft(), aAlphaBitmap );
    mpAlphaVDev->EnableMapMode( bOldMapMode );

    return res;
}

Bitmap OutputDevice::ImplBlend( Bitmap              aBmp,
                                BitmapReadAccess*   pP,
                                BitmapReadAccess*   pA,
                                const sal_Int32     nOffY,
                                const sal_Int32     nDstHeight,
                                const sal_Int32     nOffX,
                                const sal_Int32     nDstWidth,
                                const Rectangle&    aBmpRect,
                                const Size&         aOutSz,
                                const bool          bHMirr,
                                const bool          bVMirr,
                                const long*         pMapX,
                                const long*         pMapY )
{
    BitmapColor aDstCol;
    Bitmap      res;
    int         nX, nY;

    if( GetBitCount() <= 8 )
    {
        Bitmap              aDither( aBmp.GetSizePixel(), 8 );
        BitmapColor         aIndex( 0 );
        BitmapReadAccess*   pB = aBmp.AcquireReadAccess();
        BitmapWriteAccess*  pW = aDither.AcquireWriteAccess();

        if( pB && pP && pA && pW )
        {
            int nOutY;

            for( nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
            {
                const long nMapY = pMapY[ nY ];
                const long nModY = ( nOutY & 0x0FL ) << 4L;
                int nOutX;

                for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                {
                    const long  nMapX = pMapX[ nX ];
                    const sal_uLong nD = nVCLDitherLut[ nModY | ( nOutX & 0x0FL ) ];

                    aDstCol = pB->GetColor( nY, nX );
                    aDstCol.Merge( pP->GetColor( nMapY, nMapX ), pA->GetPixelIndex( nMapY, nMapX ) );
                    aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ ( nVCLLut[ aDstCol.GetRed() ] + nD ) >> 16UL ] +
                                              nVCLGLut[ ( nVCLLut[ aDstCol.GetGreen() ] + nD ) >> 16UL ] +
                                              nVCLBLut[ ( nVCLLut[ aDstCol.GetBlue() ] + nD ) >> 16UL ] ) );
                    pW->SetPixel( nY, nX, aIndex );
                }
            }
        }

        aBmp.ReleaseAccess( pB );
        aDither.ReleaseAccess( pW );
        res = aDither;
    }
    else
    {
        BitmapWriteAccess*  pB = aBmp.AcquireWriteAccess();

        bool bFastBlend = false;
        if( pP && pA && pB )
        {
            SalTwoRect aTR;
            aTR.mnSrcX      = aBmpRect.Left();
            aTR.mnSrcY      = aBmpRect.Top();
            aTR.mnSrcWidth  = aBmpRect.GetWidth();
            aTR.mnSrcHeight = aBmpRect.GetHeight();
            aTR.mnDestX     = nOffX;
            aTR.mnDestY     = nOffY;
            aTR.mnDestWidth = aOutSz.Width();
            aTR.mnDestHeight= aOutSz.Height();

            if( !bHMirr && !bVMirr )
                bFastBlend = ImplFastBitmapBlending( *pB,*pP,*pA, aTR );
        }

        if( pP && pA && pB && !bFastBlend )
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
        res = aBmp;
    }

    return res;
}

void OutputDevice::ImplDrawAlpha( const Bitmap& rBmp, const AlphaMask& rAlpha,
                                  const Point& rDestPt, const Size& rDestSize,
                                  const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    const Point aNullPt;
    Point       aOutPt( LogicToPixel( rDestPt ) );
    Size        aOutSz( LogicToPixel( rDestSize ) );
    Rectangle   aDstRect( aNullPt, GetOutputSizePixel() );
    const bool  bHMirr = aOutSz.Width() < 0;
    const bool  bVMirr = aOutSz.Height() < 0;

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
        bool bNativeAlpha = false;
        static const char* pDisableNative = getenv( "SAL_DISABLE_NATIVE_ALPHA");
        // #i83087# Naturally, system alpha blending cannot work with
        // separate alpha VDev
        bool bTryDirectPaint(!mpAlphaVDev && !pDisableNative && !bHMirr && !bVMirr);

#ifdef WNT
        if(bTryDirectPaint)
        {
            // only paint direct when no scaling and no MapMode, else the
            // more expensive conversions may be done for short-time Bitmap/BitmapEx
            // used for buffering only
            if(!IsMapMode() && rSrcSizePixel.Width() == aOutSz.Width() && rSrcSizePixel.Height() == aOutSz.Height())
            {
                bTryDirectPaint = false;
            }
        }
#endif

        if(bTryDirectPaint)
        {
            Point aRelPt = aOutPt + Point( mnOutOffX, mnOutOffY );
            SalTwoRect aTR = {
                rSrcPtPixel.X(), rSrcPtPixel.Y(),
                rSrcSizePixel.Width(), rSrcSizePixel.Height(),
                aRelPt.X(), aRelPt.Y(),
                aOutSz.Width(), aOutSz.Height()
            };
            SalBitmap* pSalSrcBmp = rBmp.ImplGetImpBitmap()->ImplGetSalBitmap();
            SalBitmap* pSalAlphaBmp = rAlpha.ImplGetImpBitmap()->ImplGetSalBitmap();
            bNativeAlpha = mpGraphics->DrawAlphaBitmap( aTR, *pSalSrcBmp, *pSalAlphaBmp, this );
        }

        VirtualDevice* pOldVDev = mpAlphaVDev;

        Rectangle aBmpRect( aNullPt, rBmp.GetSizePixel() );
        if( !bNativeAlpha
                &&  !aBmpRect.Intersection( Rectangle( rSrcPtPixel, rSrcSizePixel ) ).IsEmpty() )
        {
            GDIMetaFile*    pOldMetaFile = mpMetaFile;
            const bool      bOldMap = mbMap;
            mpMetaFile = NULL; // fdo#55044 reset before GetBitmap!
            mbMap = false;
            Bitmap          aBmp( GetBitmap( aDstRect.TopLeft(), aDstRect.GetSize() ) );

            // #109044# The generated bitmap need not necessarily be
            // of aDstRect dimensions, it's internally clipped to
            // window bounds. Thus, we correct the dest size here,
            // since we later use it (in nDstWidth/Height) for pixel
            // access)
            // #i38887# reading from screen may sometimes fail
            if( aBmp.ImplGetImpBitmap() )
                aDstRect.SetSize( aBmp.GetSizePixel() );

            BitmapColor     aDstCol;
            const long      nSrcWidth = aBmpRect.GetWidth(), nSrcHeight = aBmpRect.GetHeight();
            const long      nDstWidth = aDstRect.GetWidth(), nDstHeight = aDstRect.GetHeight();
            const long      nOutWidth = aOutSz.Width(), nOutHeight = aOutSz.Height();
            // calculate offset in original bitmap
            // in RTL case this is a little more complicated since the contents of the
            // bitmap is not mirrored (it never is), however the paint region and bmp region
            // are in mirrored coordinates, so the intersection of (aOutPt,aOutSz) with these
            // is content wise somewhere else and needs to take mirroring into account
            const long      nOffX = IsRTLEnabled()
                                    ? aOutSz.Width() - aDstRect.GetWidth() - (aDstRect.Left() - aOutPt.X())
                                    : aDstRect.Left() - aOutPt.X(),
                            nOffY = aDstRect.Top() - aOutPt.Y();
            long            nX, nOutX, nY, nOutY;
            long            nMirrOffX = 0;
            long            nMirrOffY = 0;
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

            // #i38887# reading from screen may sometimes fail
            if( aBmp.ImplGetImpBitmap() )
            {
                Bitmap aTmp;

                if( mpAlphaVDev )
                {
                    aTmp = ImplBlendWithAlpha(
                        aBmp,pP,pA,
                        aDstRect,
                        nOffY,nDstHeight,
                        nOffX,nDstWidth,
                        pMapX,pMapY );
                }
                else
                {
                    aTmp = ImplBlend(
                        aBmp,pP,pA,
                        nOffY,nDstHeight,
                        nOffX,nDstWidth,
                        aBmpRect,aOutSz,
                        bHMirr,bVMirr,
                        pMapX,pMapY );
                }

                // #110958# Disable alpha VDev, we're doing the necessary
                // stuff explicitly furher below
                if( mpAlphaVDev )
                    mpAlphaVDev = NULL;

                DrawBitmap( aDstRect.TopLeft(),
                            aTmp );

                // #110958# Enable alpha VDev again
                mpAlphaVDev = pOldVDev;
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
        sal_uLong   nMirrFlags = 0UL;

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
        long            nX, nY; // , nWorkX, nWorkY, nWorkWidth, nWorkHeight;
        long*           pMapX = new long[ nSrcWidth + 1 ];
        long*           pMapY = new long[ nSrcHeight + 1 ];
        const bool      bOldMap = mbMap;

        mbMap = false;

        // create forward mapping tables
        for( nX = 0L; nX <= nSrcWidth; nX++ )
            pMapX[ nX ] = aDestPt.X() + FRound( (double) aDestSz.Width() * nX / nSrcWidth );

        for( nY = 0L; nY <= nSrcHeight; nY++ )
            pMapY[ nY ] = aDestPt.Y() + FRound( (double) aDestSz.Height() * nY / nSrcHeight );

        // walk through all rectangles of mask
        const Region aWorkRgn(aMask.CreateRegion(COL_BLACK, Rectangle(Point(), aMask.GetSizePixel())));
        RectangleVector aRectangles;
        aWorkRgn.GetRegionRectangles(aRectangles);

        for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            const Point aMapPt(pMapX[aRectIter->Left()], pMapY[aRectIter->Top()]);
            const Size aMapSz(
                pMapX[aRectIter->Right() + 1] - aMapPt.X(),      // pMapX[L + W] -> L + ((R - L) + 1) -> R + 1
                pMapY[aRectIter->Bottom() + 1] - aMapPt.Y());    // same for Y
            Bitmap aBandBmp(aPaint);

            aBandBmp.Crop(*aRectIter);
            ImplDrawBitmap(aMapPt, aMapSz, Point(), aBandBmp.GetSizePixel(), aBandBmp, META_BMPSCALEPART_ACTION);
        }

        //Region          aWorkRgn( aMask.CreateRegion( COL_BLACK, Rectangle( Point(), aMask.GetSizePixel() ) ) );
        //ImplRegionInfo    aInfo;
        //sal_Bool            bRgnRect = aWorkRgn.ImplGetFirstRect( aInfo, nWorkX, nWorkY, nWorkWidth, nWorkHeight );
        //
        //while( bRgnRect )
        //{
        //  Bitmap          aBandBmp( aPaint );
        //    const Rectangle aBandRect( Point( nWorkX, nWorkY ), Size( nWorkWidth, nWorkHeight ) );
        //    const Point     aMapPt( pMapX[ nWorkX ], pMapY[ nWorkY ] );
        //    const Size      aMapSz( pMapX[ nWorkX + nWorkWidth ] - aMapPt.X(), pMapY[ nWorkY + nWorkHeight ] - aMapPt.Y() );
        //
        //  aBandBmp.Crop( aBandRect );
        //    ImplDrawBitmap( aMapPt, aMapSz, Point(), aBandBmp.GetSizePixel(), aBandBmp, META_BMPSCALEPART_ACTION );
        //    bRgnRect = aWorkRgn.ImplGetNextRect( aInfo, nWorkX, nWorkY, nWorkWidth, nWorkHeight );
        //}

        mbMap = bOldMap;

        delete[] pMapX;
        delete[] pMapY;
    }
}

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
        sal_uLong   nMirrFlags = 0UL;

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
        long            nX, nY; //, nWorkX, nWorkY, nWorkWidth, nWorkHeight;
        long*           pMapX = new long[ nSrcWidth + 1 ];
        long*           pMapY = new long[ nSrcHeight + 1 ];
        GDIMetaFile*    pOldMetaFile = mpMetaFile;
        const bool      bOldMap = mbMap;

        mpMetaFile = NULL;
        mbMap = false;
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
        const Region aWorkRgn(aMask.CreateRegion(COL_BLACK, Rectangle(Point(), aMask.GetSizePixel())));
        RectangleVector aRectangles;
        aWorkRgn.GetRegionRectangles(aRectangles);

        for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            const Point aMapPt(pMapX[aRectIter->Left()], pMapY[aRectIter->Top()]);
            const Size aMapSz(
                pMapX[aRectIter->Right() + 1] - aMapPt.X(),      // pMapX[L + W] -> L + ((R - L) + 1) -> R + 1
                pMapY[aRectIter->Bottom() + 1] - aMapPt.Y());    // same for Y

            DrawRect(Rectangle(aMapPt, aMapSz));
        }

        //Region          aWorkRgn( aMask.CreateRegion( COL_BLACK, Rectangle( Point(), aMask.GetSizePixel() ) ) );
        //ImplRegionInfo    aInfo;
        //sal_Bool            bRgnRect = aWorkRgn.ImplGetFirstRect( aInfo, nWorkX, nWorkY, nWorkWidth, nWorkHeight );
        //
        //while( bRgnRect )
        //{
        //    const Point aMapPt( pMapX[ nWorkX ], pMapY[ nWorkY ] );
        //    const Size  aMapSz( pMapX[ nWorkX + nWorkWidth ] - aMapPt.X(), pMapY[ nWorkY + nWorkHeight ] - aMapPt.Y() );
        //
        //  DrawRect( Rectangle( aMapPt, aMapSz ) );
        //  bRgnRect = aWorkRgn.ImplGetNextRect( aInfo, nWorkX, nWorkY, nWorkWidth, nWorkHeight );
        //}

        Pop();
        delete[] pMapX;
        delete[] pMapY;
        mbMap = bOldMap;
        mpMetaFile = pOldMetaFile;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
