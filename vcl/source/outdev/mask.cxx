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

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>
#include <impbmp.hxx>
#include <outdata.hxx>

extern const sal_uLong nVCLRLut[ 6 ];
extern const sal_uLong nVCLGLut[ 6 ];
extern const sal_uLong nVCLBLut[ 6 ];
extern const sal_uLong nVCLDitherLut[ 256 ];
extern const sal_uLong nVCLLut[ 256 ];

void OutputDevice::DrawMask( const Point& rDestPt,
                             const Bitmap& rBitmap, const Color& rMaskColor )
{
    const Size aSizePix( rBitmap.GetSizePixel() );
    DrawMask( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmap, rMaskColor, META_MASK_ACTION );
}

void OutputDevice::DrawMask( const Point& rDestPt, const Size& rDestSize,
                             const Bitmap& rBitmap, const Color& rMaskColor )
{
    DrawMask( rDestPt, rDestSize, Point(), rBitmap.GetSizePixel(), rBitmap, rMaskColor, META_MASKSCALE_ACTION );
}

void OutputDevice::DrawMask( const Point& rDestPt, const Size& rDestSize,
                             const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                             const Bitmap& rBitmap, const Color& rMaskColor,
                             const sal_uLong nAction )
{
    if( ImplIsRecordLayout() )
        return;

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

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics )
        if ( !AcquireGraphics() )
            return;

    if ( mbInitClipRegion )
        ImplInitClipRegion();

    if ( mbOutputClipped )
        return;

    ApplyMask( rBitmap, rMaskColor, rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );

}

void OutputDevice::ApplyMask( const Bitmap& rMask, const Color& rMaskColor,
                              const Point& rDestPt, const Size& rDestSize,
                              const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    const ImpBitmap* pImpBmp = rMask.ImplGetImpBitmap();
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
        const sal_uLong nMirrFlags = AdjustTwoRect( aPosAry, pImpBmp->ImplGetSize() );

        // check if output is necessary
        if( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
        {

            if( nMirrFlags )
            {
                Bitmap aTmp( rMask );
                aTmp.Mirror( nMirrFlags );
                mpGraphics->DrawMask( aPosAry, *aTmp.ImplGetImpBitmap()->ImplGetSalBitmap(),
                                      ImplColorToSal( rMaskColor ) , this);
            }
            else
                mpGraphics->DrawMask( aPosAry, *pImpBmp->ImplGetSalBitmap(),
                                      ImplColorToSal( rMaskColor ), this );

        }
    }

    // TODO: Use mask here
    if( mpAlphaVDev )
    {
        const Bitmap& rAlphaMask( rMask.CreateMask( rMaskColor ) );

        // #i25167# Restrict mask painting to _opaque_ areas
        // of the mask, otherwise we spoil areas where no
        // bitmap content was ever visible. Interestingly
        // enough, this can be achieved by taking the mask as
        // the transparency mask of itself
        mpAlphaVDev->DrawBitmapEx( rDestPt,
                                   rDestSize,
                                   rSrcPtPixel,
                                   rSrcSizePixel,
                                   BitmapEx( rAlphaMask, rMask ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
