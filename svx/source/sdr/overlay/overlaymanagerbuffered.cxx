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

#include <sdr/overlay/overlaymanagerbuffered.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>
#include <tools/fract.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/cursor.hxx>
#include <vcl/dibtools.hxx>


namespace sdr
{
    namespace overlay
    {
        void OverlayManagerBuffered::ImpPrepareBufferDevice()
        {
            // compare size of mpBufferDevice with size of visible area
            if(mpBufferDevice->GetOutputSizePixel() != getOutputDevice().GetOutputSizePixel())
            {
                // set new buffer size, copy as much content as possible (use bool parameter for vcl).
                // Newly uncovered regions will be repainted.
                mpBufferDevice->SetOutputSizePixel(getOutputDevice().GetOutputSizePixel(), false);
            }

            // compare the MapModes for zoom/scroll changes
            if(mpBufferDevice->GetMapMode() != getOutputDevice().GetMapMode())
            {
                const bool bZoomed(
                    mpBufferDevice->GetMapMode().GetScaleX() != getOutputDevice().GetMapMode().GetScaleX()
                    || mpBufferDevice->GetMapMode().GetScaleY() != getOutputDevice().GetMapMode().GetScaleY());

                if(!bZoomed)
                {
                    const Point& rOriginOld = mpBufferDevice->GetMapMode().GetOrigin();
                    const Point& rOriginNew = getOutputDevice().GetMapMode().GetOrigin();
                    const bool bScrolled(rOriginOld != rOriginNew);

                    if(bScrolled)
                    {
                        // get pixel bounds
                        const Point aOriginOldPixel(mpBufferDevice->LogicToPixel(rOriginOld));
                        const Point aOriginNewPixel(mpBufferDevice->LogicToPixel(rOriginNew));
                        const Size aOutputSizePixel(mpBufferDevice->GetOutputSizePixel());

                        // remember and switch off MapMode
                        const bool bMapModeWasEnabled(mpBufferDevice->IsMapModeEnabled());
                        mpBufferDevice->EnableMapMode(false);

                        // scroll internally buffered stuff
                        const Point aDestinationOffsetPixel(aOriginNewPixel - aOriginOldPixel);
                        mpBufferDevice->DrawOutDev(
                            aDestinationOffsetPixel, aOutputSizePixel, // destination
                            Point(), aOutputSizePixel); // source

                        // restore MapMode
                        mpBufferDevice->EnableMapMode(bMapModeWasEnabled);

                        // scroll remembered region, too.
                        if(!maBufferRememberedRangePixel.isEmpty())
                        {
                            const basegfx::B2IPoint aIPointDestinationOffsetPixel(aDestinationOffsetPixel.X(), aDestinationOffsetPixel.Y());
                            const basegfx::B2IPoint aNewMinimum(maBufferRememberedRangePixel.getMinimum() + aIPointDestinationOffsetPixel);
                            const basegfx::B2IPoint aNewMaximum(maBufferRememberedRangePixel.getMaximum() + aIPointDestinationOffsetPixel);
                            maBufferRememberedRangePixel = basegfx::B2IRange(aNewMinimum, aNewMaximum);
                        }
                    }
                }

                // copy new MapMode
                mpBufferDevice->SetMapMode(getOutputDevice().GetMapMode());
            }

            // #i29186#
            mpBufferDevice->SetDrawMode(getOutputDevice().GetDrawMode());
            mpBufferDevice->SetSettings(getOutputDevice().GetSettings());
            mpBufferDevice->SetAntialiasing(getOutputDevice().GetAntialiasing());
        }

        void OverlayManagerBuffered::ImpRestoreBackground() const
        {
            const tools::Rectangle aRegionRectanglePixel(
                maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY());
            const vcl::Region aRegionPixel(aRegionRectanglePixel);

            ImpRestoreBackground(aRegionPixel);
        }

        void OverlayManagerBuffered::ImpRestoreBackground(const vcl::Region& rRegionPixel) const
        {
            // MapModes off
            const bool bMapModeWasEnabledDest(getOutputDevice().IsMapModeEnabled());
            const bool bMapModeWasEnabledSource(mpBufferDevice->IsMapModeEnabled());
            getOutputDevice().EnableMapMode(false);
            const_cast<OverlayManagerBuffered*>(this)->mpBufferDevice->EnableMapMode(false);

            // local region
            RectangleVector aRectangles;
            rRegionPixel.GetRegionRectangles(aRectangles);

            for(const auto& rRect : aRectangles)
            {
                // restore the area
                const Point aTopLeft(rRect.TopLeft());
                const Size aSize(rRect.GetSize());

                getOutputDevice().DrawOutDev(
                    aTopLeft, aSize, // destination
                    aTopLeft, aSize, // source
                    *mpBufferDevice);
            }

            // restore MapModes
            getOutputDevice().EnableMapMode(bMapModeWasEnabledDest);
            const_cast<OverlayManagerBuffered*>(this)->mpBufferDevice->EnableMapMode(bMapModeWasEnabledSource);
        }

        void OverlayManagerBuffered::ImpSaveBackground(const vcl::Region& rRegion, OutputDevice* pPreRenderDevice)
        {
            // prepare source
            OutputDevice& rSource = pPreRenderDevice ? *pPreRenderDevice : getOutputDevice();

            // Ensure buffer is valid
            ImpPrepareBufferDevice();

            // build region which needs to be copied
            vcl::Region aRegion(rSource.LogicToPixel(rRegion));

            // limit to PaintRegion if it's a window. This will be evtl. the expanded one,
            // but always the exact redraw area
            if(OUTDEV_WINDOW == rSource.GetOutDevType())
            {
                vcl::Window& rWindow = static_cast<vcl::Window&>(rSource);
                vcl::Region aPaintRegionPixel = rWindow.LogicToPixel(rWindow.GetPaintRegion());
                aRegion.Intersect(aPaintRegionPixel);

                // #i72754# Make sure content is completely rendered, the window
                // will be used as source of a DrawOutDev soon
                rWindow.Flush();
            }

            // also limit to buffer size
            const tools::Rectangle aBufferDeviceRectanglePixel(Point(), mpBufferDevice->GetOutputSizePixel());
            aRegion.Intersect(aBufferDeviceRectanglePixel);

            // MapModes off
            const bool bMapModeWasEnabledDest(rSource.IsMapModeEnabled());
            const bool bMapModeWasEnabledSource(mpBufferDevice->IsMapModeEnabled());
            rSource.EnableMapMode(false);
            mpBufferDevice->EnableMapMode(false);

            // prepare to iterate over the rectangles from the region in pixels
            RectangleVector aRectangles;
            aRegion.GetRegionRectangles(aRectangles);

            for(const auto& rRect : aRectangles)
            {
                // for each rectangle, save the area
                const Point aTopLeft(rRect.TopLeft());
                const Size aSize(rRect.GetSize());

                mpBufferDevice->DrawOutDev(
                    aTopLeft, aSize, // destination
                    aTopLeft, aSize, // source
                    rSource);
            }

            // restore MapModes
            rSource.EnableMapMode(bMapModeWasEnabledDest);
            mpBufferDevice->EnableMapMode(bMapModeWasEnabledSource);
        }

        IMPL_LINK_NOARG(OverlayManagerBuffered, ImpBufferTimerHandler, Timer*, void)
        {
            //Resolves: fdo#46728 ensure this exists until end of scope
            rtl::Reference<OverlayManager> xRef(this);

            // stop timer
            maBufferIdle.Stop();

            if(!maBufferRememberedRangePixel.isEmpty())
            {
                // logic size for impDrawMember call
                basegfx::B2DRange aBufferRememberedRangeLogic(
                    maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                    maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY());
                aBufferRememberedRangeLogic.transform(getOutputDevice().GetInverseViewTransformation());

                // prepare cursor handling
                const bool bTargetIsWindow(OUTDEV_WINDOW == mrOutputDevice.GetOutDevType());
                bool bCursorWasEnabled(false);

                // #i80730# switch off VCL cursor during overlay refresh
                if(bTargetIsWindow)
                {
                    vcl::Window& rWindow = static_cast< vcl::Window& >(mrOutputDevice);
                    vcl::Cursor* pCursor = rWindow.GetCursor();

                    if(pCursor && pCursor->IsVisible())
                    {
                        pCursor->Hide();
                        bCursorWasEnabled = true;
                    }
                }

                // refresh with prerendering
                {
                    // #i73602# ensure valid and sized mpOutputBufferDevice
                    const Size aDestinationSizePixel(mpBufferDevice->GetOutputSizePixel());
                    const Size aOutputBufferSizePixel(mpOutputBufferDevice->GetOutputSizePixel());

                    if(aDestinationSizePixel != aOutputBufferSizePixel)
                    {
                        mpOutputBufferDevice->SetOutputSizePixel(aDestinationSizePixel);
                    }

                    mpOutputBufferDevice->SetMapMode(getOutputDevice().GetMapMode());
                    mpOutputBufferDevice->EnableMapMode(false);
                    mpOutputBufferDevice->SetDrawMode(mpBufferDevice->GetDrawMode());
                    mpOutputBufferDevice->SetSettings(mpBufferDevice->GetSettings());
                    mpOutputBufferDevice->SetAntialiasing(mpBufferDevice->GetAntialiasing());

                    // calculate sizes
                    tools::Rectangle aRegionRectanglePixel(
                        maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                        maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY());

                    // truncate aRegionRectanglePixel to destination pixel size, more does
                    // not need to be prepared since destination is a buffer for a window. So,
                    // maximum size indirectly shall be limited to getOutputDevice().GetOutputSizePixel()
                    if(aRegionRectanglePixel.Left() < 0)
                    {
                        aRegionRectanglePixel.SetLeft( 0 );
                    }

                    if(aRegionRectanglePixel.Top() < 0)
                    {
                        aRegionRectanglePixel.SetTop( 0 );
                    }

                    if(aRegionRectanglePixel.Right() > aDestinationSizePixel.getWidth())
                    {
                        aRegionRectanglePixel.SetRight( aDestinationSizePixel.getWidth() );
                    }

                    if(aRegionRectanglePixel.Bottom() > aDestinationSizePixel.getHeight())
                    {
                        aRegionRectanglePixel.SetBottom( aDestinationSizePixel.getHeight() );
                    }

                    // get sizes
                    const Point aTopLeft(aRegionRectanglePixel.TopLeft());
                    const Size aSize(aRegionRectanglePixel.GetSize());

                    {
                        const bool bMapModeWasEnabledDest(mpBufferDevice->IsMapModeEnabled());
                        mpBufferDevice->EnableMapMode(false);

                        mpOutputBufferDevice->DrawOutDev(
                            aTopLeft, aSize, // destination
                            aTopLeft, aSize, // source
                            *mpBufferDevice);

                        // restore MapModes
                        mpBufferDevice->EnableMapMode(bMapModeWasEnabledDest);
                    }

                    // paint overlay content for remembered region, use
                    // method from base class directly
                    mpOutputBufferDevice->EnableMapMode();
                    OverlayManager::ImpDrawMembers(aBufferRememberedRangeLogic, *mpOutputBufferDevice);
                    mpOutputBufferDevice->EnableMapMode(false);

                    // copy to output
                    {
                        const bool bMapModeWasEnabledDest(getOutputDevice().IsMapModeEnabled());
                        getOutputDevice().EnableMapMode(false);

                        getOutputDevice().DrawOutDev(
                            aTopLeft, aSize, // destination
                            aTopLeft, aSize, // source
                            *mpOutputBufferDevice);

                        // debug
                        /*getOutputDevice().SetLineCOL_RED);
                        getOutputDevice().SetFillColor();
                        getOutputDevice().DrawRect(Rectangle(aTopLeft, aSize));*/

                        // restore MapModes
                        getOutputDevice().EnableMapMode(bMapModeWasEnabledDest);
                    }
                }

                // VCL hack for transparent child windows
                // Problem is e.g. a radiobutton form control in life mode. The used window
                // is a transparence vcl childwindow. This flag only allows the parent window to
                // paint into the child windows area, but there is no mechanism which takes
                // care for a repaint of the child window. A transparent child window is NOT
                // a window which always keeps it's content consistent over the parent, but it's
                // more like just a paint flag for the parent.
                // To get the update, the windows in question are updated manually here.
                if(bTargetIsWindow)
                {
                    vcl::Window& rWindow = static_cast< vcl::Window& >(mrOutputDevice);

                    const tools::Rectangle aRegionRectanglePixel(
                        maBufferRememberedRangePixel.getMinX(),
                        maBufferRememberedRangePixel.getMinY(),
                        maBufferRememberedRangePixel.getMaxX(),
                        maBufferRememberedRangePixel.getMaxY());
                    PaintTransparentChildren(rWindow, aRegionRectanglePixel);
                }

                // #i80730# restore visibility of VCL cursor
                if(bCursorWasEnabled)
                {
                    vcl::Window& rWindow = static_cast< vcl::Window& >(mrOutputDevice);
                    vcl::Cursor* pCursor = rWindow.GetCursor();

                    if(pCursor)
                    {
                        // check if cursor still exists. It may have been deleted from someone
                        pCursor->Show();
                    }
                }

                // forget remembered Region
                maBufferRememberedRangePixel.reset();
            }
        }

        OverlayManagerBuffered::OverlayManagerBuffered(
            OutputDevice& rOutputDevice)
        :   OverlayManager(rOutputDevice),
            mpBufferDevice(VclPtr<VirtualDevice>::Create()),
            mpOutputBufferDevice(VclPtr<VirtualDevice>::Create()),
            maBufferIdle("sdr overlay OverlayManagerBuffered Idle")
        {
            // Init timer
            maBufferIdle.SetPriority( TaskPriority::POST_PAINT );
            maBufferIdle.SetInvokeHandler(LINK(this, OverlayManagerBuffered, ImpBufferTimerHandler));
            maBufferIdle.SetDebugName( "sdr::overlay::OverlayManagerBuffered maBufferIdle" );
        }

        rtl::Reference<OverlayManager> OverlayManagerBuffered::create(
            OutputDevice& rOutputDevice)
        {
            return rtl::Reference<OverlayManager>(new OverlayManagerBuffered(rOutputDevice));
        }

        OverlayManagerBuffered::~OverlayManagerBuffered()
        {
            // Clear timer
            maBufferIdle.Stop();

            if(!maBufferRememberedRangePixel.isEmpty())
            {
                // Restore all rectangles for remembered region from buffer
                ImpRestoreBackground();
            }
        }

        void OverlayManagerBuffered::completeRedraw(const vcl::Region& rRegion, OutputDevice* pPreRenderDevice) const
        {
            if(!rRegion.IsEmpty())
            {
                // save new background
                const_cast<OverlayManagerBuffered*>(this)->ImpSaveBackground(rRegion, pPreRenderDevice);
            }

            // call parent
            OverlayManager::completeRedraw(rRegion, pPreRenderDevice);
        }

        void OverlayManagerBuffered::flush()
        {
            // call timer handler direct
            ImpBufferTimerHandler(nullptr);
        }

        void OverlayManagerBuffered::invalidateRange(const basegfx::B2DRange& rRange)
        {
            if(!rRange.isEmpty())
            {
                // buffered output, do not invalidate but use the timer
                // to trigger a timer event for refresh
                maBufferIdle.Start();

                // add the discrete range to the remembered region
                // #i75163# use double precision and floor/ceil rounding to get overlapped pixel region, even
                // when the given logic region has a width/height of 0.0. This does NOT work with LogicToPixel
                // since it just transforms the top left and bottom right points equally without taking
                // discrete pixel coverage into account. An empty B2DRange and thus empty logic Rectangle translated
                // to an also empty discrete pixel rectangle - what is wrong.
                basegfx::B2DRange aDiscreteRange(rRange);
                aDiscreteRange.transform(getOutputDevice().GetViewTransformation());

                if(maDrawinglayerOpt.IsAntiAliasing())
                {
                    // assume AA needs one pixel more and invalidate one pixel more
                    const double fDiscreteOne(getDiscreteOne());
                    const basegfx::B2IPoint aTopLeft(
                        static_cast<sal_Int32>(floor(aDiscreteRange.getMinX() - fDiscreteOne)),
                        static_cast<sal_Int32>(floor(aDiscreteRange.getMinY() - fDiscreteOne)));
                    const basegfx::B2IPoint aBottomRight(
                        static_cast<sal_Int32>(ceil(aDiscreteRange.getMaxX() + fDiscreteOne)),
                        static_cast<sal_Int32>(ceil(aDiscreteRange.getMaxY() + fDiscreteOne)));

                    maBufferRememberedRangePixel.expand(aTopLeft);
                    maBufferRememberedRangePixel.expand(aBottomRight);
                }
                else
                {
                    const basegfx::B2IPoint aTopLeft(static_cast<sal_Int32>(floor(aDiscreteRange.getMinX())), static_cast<sal_Int32>(floor(aDiscreteRange.getMinY())));
                    const basegfx::B2IPoint aBottomRight(static_cast<sal_Int32>(ceil(aDiscreteRange.getMaxX())), static_cast<sal_Int32>(ceil(aDiscreteRange.getMaxY())));

                    maBufferRememberedRangePixel.expand(aTopLeft);
                    maBufferRememberedRangePixel.expand(aBottomRight);
                }
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
