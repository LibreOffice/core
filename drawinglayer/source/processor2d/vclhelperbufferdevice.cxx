/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclhelperbufferdevice.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
                SvFileStream aNew((const String&)String(ByteString( "c:\\alpha.bmp" ), RTL_TEXTENCODING_UTF8), STREAM_WRITE|STREAM_TRUNC);
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

    VirtualDevice& impBufferDevice::getAlpha()
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
