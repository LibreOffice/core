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
#include <boost/scoped_array.hpp>

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

    ClipToPaintRegion(aDstRect);

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
            // The scaling in this code path produces really ugly results - it
            // does the most trivial scaling with no smoothing.

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
            boost::scoped_array<long> pMapX(new long[ nDstWidth ]);
            boost::scoped_array<long> pMapY(new long[ nDstHeight ]);

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
                        pMapX.get(),pMapY.get() );
                }
                else
                {
                    aTmp = ImplBlend(
                        aBmp,pP,pA,
                        nOffY,nDstHeight,
                        nOffX,nDstWidth,
                        aBmpRect,aOutSz,
                        bHMirr,bVMirr,
                        pMapX.get(),pMapY.get() );
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

            mbMap = bOldMap;
            mpMetaFile = pOldMetaFile;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
