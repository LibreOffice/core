/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <vclhelperbufferdevice.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vcl/bitmapex.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/stream.hxx>

//////////////////////////////////////////////////////////////////////////////
// support for rendering Bitmap and BitmapEx contents

namespace drawinglayer
{
    impBufferDevice::impBufferDevice(
        OutputDevice& rOutDev,
        const basegfx::B2DRange& rRange,
        bool bAddOffsetToMapping)
    :   mrOutDev(rOutDev),
        maContent(rOutDev),
        mpMask(0L),
        mpAlpha(0L)
    {
        basegfx::B2DRange aRangePixel(rRange);
        aRangePixel.transform(rOutDev.GetViewTransformation());
        const Rectangle aRectPixel(
            (sal_Int32)floor(aRangePixel.getMinX()), (sal_Int32)floor(aRangePixel.getMinY()),
            (sal_Int32)ceil(aRangePixel.getMaxX()), (sal_Int32)ceil(aRangePixel.getMaxY()));
        const Point aEmptyPoint;
        maDestPixel = Rectangle(aEmptyPoint, rOutDev.GetOutputSizePixel());
        maDestPixel.Intersection(aRectPixel);

        if(isVisible())
        {
            maContent.SetOutputSizePixel(maDestPixel.GetSize(), false);

            // #i93485# assert when copying from window to VDev is used
            OSL_ENSURE(rOutDev.GetOutDevType() != OUTDEV_WINDOW,
                "impBufferDevice render helper: Copying from Window to VDev, this should be avoided (!)");

            const bool bWasEnabledSrc(rOutDev.IsMapModeEnabled());
            rOutDev.EnableMapMode(false);
            maContent.DrawOutDev(aEmptyPoint, maDestPixel.GetSize(), maDestPixel.TopLeft(), maDestPixel.GetSize(), rOutDev);
            rOutDev.EnableMapMode(bWasEnabledSrc);

            MapMode aNewMapMode(rOutDev.GetMapMode());

            if(bAddOffsetToMapping)
            {
                const Point aLogicTopLeft(rOutDev.PixelToLogic(maDestPixel.TopLeft()));
                aNewMapMode.SetOrigin(Point(-aLogicTopLeft.X(), -aLogicTopLeft.Y()));
            }

            maContent.SetMapMode(aNewMapMode);

            // copy AA flag for new target
            maContent.SetAntialiasing(mrOutDev.GetAntialiasing());
        }
    }

    impBufferDevice::~impBufferDevice()
    {
        delete mpMask;
        delete mpAlpha;
    }

    void impBufferDevice::paint(double fTrans)
    {
        const Point aEmptyPoint;
        const Size aSizePixel(maContent.GetOutputSizePixel());
        const bool bWasEnabledDst(mrOutDev.IsMapModeEnabled());

        mrOutDev.EnableMapMode(false);
        maContent.EnableMapMode(false);
        Bitmap aContent(maContent.GetBitmap(aEmptyPoint, aSizePixel));

        if(mpAlpha)
        {
            mpAlpha->EnableMapMode(false);
            const AlphaMask aAlphaMask(mpAlpha->GetBitmap(aEmptyPoint, aSizePixel));

            mrOutDev.DrawBitmapEx(maDestPixel.TopLeft(), BitmapEx(aContent, aAlphaMask));
        }
        else if(mpMask)
        {
            mpMask->EnableMapMode(false);
            const Bitmap aMask(mpMask->GetBitmap(aEmptyPoint, aSizePixel));

            mrOutDev.DrawBitmapEx(maDestPixel.TopLeft(), BitmapEx(aContent, aMask));
        }
        else if(0.0 != fTrans)
        {
            sal_uInt8 nMaskValue((sal_uInt8)basegfx::fround(fTrans * 255.0));
            const AlphaMask aAlphaMask(aSizePixel, &nMaskValue);
            mrOutDev.DrawBitmapEx(maDestPixel.TopLeft(), BitmapEx(aContent, aAlphaMask));
        }
        else
        {
            mrOutDev.DrawBitmap(maDestPixel.TopLeft(), aContent);
        }

        mrOutDev.EnableMapMode(bWasEnabledDst);
    }

    VirtualDevice& impBufferDevice::getMask()
    {
        if(!mpMask)
        {
            mpMask = new VirtualDevice(mrOutDev, 1);
            mpMask->SetOutputSizePixel(maDestPixel.GetSize(), true);
            mpMask->SetMapMode(maContent.GetMapMode());

            // do NOT copy AA flag for mask!
        }

        return *mpMask;
    }

    VirtualDevice& impBufferDevice::getTransparence()
    {
        if(!mpAlpha)
        {
            mpAlpha = new VirtualDevice();
            mpAlpha->SetOutputSizePixel(maDestPixel.GetSize(), true);
            mpAlpha->SetMapMode(maContent.GetMapMode());

            // copy AA flag for new target; masking needs to be smooth
            mpAlpha->SetAntialiasing(maContent.GetAntialiasing());
        }

        return *mpAlpha;
    }
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
