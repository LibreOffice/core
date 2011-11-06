/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

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
        static bool bDoSaveForVisualControl(false);

        mrOutDev.EnableMapMode(false);
        maContent.EnableMapMode(false);
        Bitmap aContent(maContent.GetBitmap(aEmptyPoint, aSizePixel));

        if(bDoSaveForVisualControl)
        {
            SvFileStream aNew((const String&)String(ByteString( "c:\\content.bmp" ), RTL_TEXTENCODING_UTF8), STREAM_WRITE|STREAM_TRUNC);
            aNew << aContent;
        }

        if(mpAlpha)
        {
            mpAlpha->EnableMapMode(false);
            const AlphaMask aAlphaMask(mpAlpha->GetBitmap(aEmptyPoint, aSizePixel));

            if(bDoSaveForVisualControl)
            {
                SvFileStream aNew((const String&)String(ByteString( "c:\\transparence.bmp" ), RTL_TEXTENCODING_UTF8), STREAM_WRITE|STREAM_TRUNC);
                aNew << aAlphaMask.GetBitmap();
            }

            mrOutDev.DrawBitmapEx(maDestPixel.TopLeft(), BitmapEx(aContent, aAlphaMask));
        }
        else if(mpMask)
        {
            mpMask->EnableMapMode(false);
            const Bitmap aMask(mpMask->GetBitmap(aEmptyPoint, aSizePixel));

            if(bDoSaveForVisualControl)
            {
                SvFileStream aNew((const String&)String(ByteString( "c:\\mask.bmp" ), RTL_TEXTENCODING_UTF8), STREAM_WRITE|STREAM_TRUNC);
                aNew << aMask;
            }

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

//////////////////////////////////////////////////////////////////////////////
// eof
