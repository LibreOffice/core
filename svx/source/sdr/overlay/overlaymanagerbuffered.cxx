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

#include <svx/sdr/overlay/overlaymanagerbuffered.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/cursor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayManagerBuffered::ImpPrepareBufferDevice()
        {
            // compare size of maBufferDevice with size of visible area
            if(maBufferDevice.GetOutputSizePixel() != getOutputDevice().GetOutputSizePixel())
            {
                // set new buffer size, copy as much content as possible (use bool parameter for vcl).
                // Newly uncovered regions will be repainted.
                maBufferDevice.SetOutputSizePixel(getOutputDevice().GetOutputSizePixel(), false);
            }

            // compare the MapModes for zoom/scroll changes
            if(maBufferDevice.GetMapMode() != getOutputDevice().GetMapMode())
            {
                const bool bZoomed(
                    maBufferDevice.GetMapMode().GetScaleX() != getOutputDevice().GetMapMode().GetScaleX()
                    || maBufferDevice.GetMapMode().GetScaleY() != getOutputDevice().GetMapMode().GetScaleY());

                if(!bZoomed)
                {
                    const Point& rOriginOld = maBufferDevice.GetMapMode().GetOrigin();
                    const Point& rOriginNew = getOutputDevice().GetMapMode().GetOrigin();
                    const bool bScrolled(rOriginOld != rOriginNew);

                    if(bScrolled)
                    {
                        // get pixel bounds
                        const Point aOriginOldPixel(maBufferDevice.LogicToPixel(rOriginOld));
                        const Point aOriginNewPixel(maBufferDevice.LogicToPixel(rOriginNew));
                        const Size aOutputSizePixel(maBufferDevice.GetOutputSizePixel());

                        // remember and switch off MapMode
                        const bool bMapModeWasEnabled(maBufferDevice.IsMapModeEnabled());
                        maBufferDevice.EnableMapMode(false);

                        // scroll internally buffered stuff
                        const Point aDestinationOffsetPixel(aOriginNewPixel - aOriginOldPixel);
                        maBufferDevice.DrawOutDev(
                            aDestinationOffsetPixel, aOutputSizePixel, // destination
                            Point(), aOutputSizePixel); // source

                        // restore MapMode
                        maBufferDevice.EnableMapMode(bMapModeWasEnabled);

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
                maBufferDevice.SetMapMode(getOutputDevice().GetMapMode());
            }

            // #i29186#
            maBufferDevice.SetDrawMode(getOutputDevice().GetDrawMode());
            maBufferDevice.SetSettings(getOutputDevice().GetSettings());
            maBufferDevice.SetAntialiasing(getOutputDevice().GetAntialiasing());
        }

        void OverlayManagerBuffered::ImpRestoreBackground() const
        {
            const Rectangle aRegionRectanglePixel(
                maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY());
            const Region aRegionPixel(aRegionRectanglePixel);

            ImpRestoreBackground(aRegionPixel);
        }

        void OverlayManagerBuffered::ImpRestoreBackground(const Region& rRegionPixel) const
        {
            // local region
            Region aRegionPixel(rRegionPixel);
            RegionHandle aRegionHandle(aRegionPixel.BeginEnumRects());
            Rectangle aRegionRectanglePixel;

            // MapModes off
            const bool bMapModeWasEnabledDest(getOutputDevice().IsMapModeEnabled());
            const bool bMapModeWasEnabledSource(maBufferDevice.IsMapModeEnabled());
            getOutputDevice().EnableMapMode(false);
            ((OverlayManagerBuffered*)this)->maBufferDevice.EnableMapMode(false);

            while(aRegionPixel.GetEnumRects(aRegionHandle, aRegionRectanglePixel))
            {
#ifdef DBG_UTIL
                // #i72754# possible graphical region test only with non-pro
                static bool bDoPaintForVisualControl(false);
                if(bDoPaintForVisualControl)
                {
                    getOutputDevice().SetLineColor(COL_LIGHTGREEN);
                    getOutputDevice().SetFillColor();
                    getOutputDevice().DrawRect(aRegionRectanglePixel);
                }
#endif

                // restore the area
                const Point aTopLeft(aRegionRectanglePixel.TopLeft());
                const Size aSize(aRegionRectanglePixel.GetSize());

                getOutputDevice().DrawOutDev(
                    aTopLeft, aSize, // destination
                    aTopLeft, aSize, // source
                    maBufferDevice);
            }

            aRegionPixel.EndEnumRects(aRegionHandle);

            // restore MapModes
            getOutputDevice().EnableMapMode(bMapModeWasEnabledDest);
            ((OverlayManagerBuffered*)this)->maBufferDevice.EnableMapMode(bMapModeWasEnabledSource);
        }

        void OverlayManagerBuffered::ImpSaveBackground(const Region& rRegion, OutputDevice* pPreRenderDevice)
        {
            // prepare source
            OutputDevice& rSource = (pPreRenderDevice) ? *pPreRenderDevice : getOutputDevice();

            // Ensure buffer is valid
            ImpPrepareBufferDevice();

            // build region which needs to be copied
            Region aRegion(rSource.LogicToPixel(rRegion));

            // limit to PaintRegion if it's a window. This will be evtl. the expanded one,
            // but always the exact redraw area
            if(OUTDEV_WINDOW == rSource.GetOutDevType())
            {
                Window& rWindow = (Window&)rSource;
                Region aPaintRegionPixel = rWindow.LogicToPixel(rWindow.GetPaintRegion());
                aRegion.Intersect(aPaintRegionPixel);

                // #i72754# Make sure content is completetly rendered, the window
                // will be used as source of a DrawOutDev soon
                rWindow.Flush();
            }

            // also limit to buffer size
            const Rectangle aBufferDeviceRectanglePixel = Rectangle(Point(), maBufferDevice.GetOutputSizePixel());
            aRegion.Intersect(aBufferDeviceRectanglePixel);

            // prepare to iterate over the rectangles from the region in pixels
            RegionHandle aRegionHandle(aRegion.BeginEnumRects());
            Rectangle aRegionRectanglePixel;

            // MapModes off
            const bool bMapModeWasEnabledDest(rSource.IsMapModeEnabled());
            const bool bMapModeWasEnabledSource(maBufferDevice.IsMapModeEnabled());
            rSource.EnableMapMode(false);
            maBufferDevice.EnableMapMode(false);

            while(aRegion.GetEnumRects(aRegionHandle, aRegionRectanglePixel))
            {
                // for each rectangle, save the area
                Point aTopLeft(aRegionRectanglePixel.TopLeft());
                Size aSize(aRegionRectanglePixel.GetSize());

                maBufferDevice.DrawOutDev(
                    aTopLeft, aSize, // destination
                    aTopLeft, aSize, // source
                    rSource);
            }

            aRegion.EndEnumRects(aRegionHandle);

            // restore MapModes
            rSource.EnableMapMode(bMapModeWasEnabledDest);
            maBufferDevice.EnableMapMode(bMapModeWasEnabledSource);
        }

        IMPL_LINK(OverlayManagerBuffered, ImpBufferTimerHandler, AutoTimer*, /*pTimer*/)
        {
            //Resolves: fdo#46728 ensure this exists until end of scope
            rtl::Reference<OverlayManager> xRef(this);

            // stop timer
            maBufferTimer.Stop();

            if(!maBufferRememberedRangePixel.isEmpty())
            {
                // logic size for impDrawMember call
                basegfx::B2DRange aBufferRememberedRangeLogic(
                    maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                    maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY());
                aBufferRememberedRangeLogic.transform(getOutputDevice().GetInverseViewTransformation());

                // prepare cursor handling
                const bool bTargetIsWindow(OUTDEV_WINDOW == rmOutputDevice.GetOutDevType());
                bool bCursorWasEnabled(false);

                // #i80730# switch off VCL cursor during overlay refresh
                if(bTargetIsWindow)
                {
                    Window& rWindow = static_cast< Window& >(rmOutputDevice);
                    Cursor* pCursor = rWindow.GetCursor();

                    if(pCursor && pCursor->IsVisible())
                    {
                        pCursor->Hide();
                        bCursorWasEnabled = true;
                    }
                }

                if(DoRefreshWithPreRendering())
                {
                    // #i73602# ensure valid and sized maOutputBufferDevice
                    const Size aDestinationSizePixel(maBufferDevice.GetOutputSizePixel());
                    const Size aOutputBufferSizePixel(maOutputBufferDevice.GetOutputSizePixel());

                    if(aDestinationSizePixel != aOutputBufferSizePixel)
                    {
                        maOutputBufferDevice.SetOutputSizePixel(aDestinationSizePixel);
                    }

                    maOutputBufferDevice.SetMapMode(getOutputDevice().GetMapMode());
                    maOutputBufferDevice.EnableMapMode(false);
                    maOutputBufferDevice.SetDrawMode(maBufferDevice.GetDrawMode());
                    maOutputBufferDevice.SetSettings(maBufferDevice.GetSettings());
                    maOutputBufferDevice.SetAntialiasing(maBufferDevice.GetAntialiasing());

                    // calculate sizes
                    Rectangle aRegionRectanglePixel(
                        maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                        maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY());

                    // truncate aRegionRectanglePixel to destination pixel size, more does
                    // not need to be prepared since destination is a buffer for a window. So,
                    // maximum size indirectly shall be limited to getOutputDevice().GetOutputSizePixel()
                    if(aRegionRectanglePixel.Left() < 0L)
                    {
                        aRegionRectanglePixel.Left() = 0L;
                    }

                    if(aRegionRectanglePixel.Top() < 0L)
                    {
                        aRegionRectanglePixel.Top() = 0L;
                    }

                    if(aRegionRectanglePixel.Right() > aDestinationSizePixel.getWidth())
                    {
                        aRegionRectanglePixel.Right() = aDestinationSizePixel.getWidth();
                    }

                    if(aRegionRectanglePixel.Bottom() > aDestinationSizePixel.getHeight())
                    {
                        aRegionRectanglePixel.Bottom() = aDestinationSizePixel.getHeight();
                    }

                    // get sizes
                    const Point aTopLeft(aRegionRectanglePixel.TopLeft());
                    const Size aSize(aRegionRectanglePixel.GetSize());

                    {
                        const bool bMapModeWasEnabledDest(maBufferDevice.IsMapModeEnabled());
                        maBufferDevice.EnableMapMode(false);

                        maOutputBufferDevice.DrawOutDev(
                            aTopLeft, aSize, // destination
                            aTopLeft, aSize, // source
                            maBufferDevice);

                        // restore MapModes
                        maBufferDevice.EnableMapMode(bMapModeWasEnabledDest);
                    }

                    // paint overlay content for remembered region, use
                    // method from base class directly
                    maOutputBufferDevice.EnableMapMode(true);
                    OverlayManager::ImpDrawMembers(aBufferRememberedRangeLogic, maOutputBufferDevice);
                    maOutputBufferDevice.EnableMapMode(false);

                    // copy to output
                    {
                        const bool bMapModeWasEnabledDest(getOutputDevice().IsMapModeEnabled());
                        getOutputDevice().EnableMapMode(false);

                        getOutputDevice().DrawOutDev(
                            aTopLeft, aSize, // destination
                            aTopLeft, aSize, // source
                            maOutputBufferDevice);

                        // debug
                        /*getOutputDevice().SetLineColor(COL_RED);
                        getOutputDevice().SetFillColor();
                        getOutputDevice().DrawRect(Rectangle(aTopLeft, aSize));*/

                        // restore MapModes
                        getOutputDevice().EnableMapMode(bMapModeWasEnabledDest);
                    }
                }
                else
                {
                    // Restore all rectangles for remembered region from buffer
                    ImpRestoreBackground();

                    // paint overlay content for remembered region, use
                    // method from base class directly
                    OverlayManager::ImpDrawMembers(aBufferRememberedRangeLogic, getOutputDevice());
                }

                // VCL hack for transparent child windows
                // Problem is e.g. a radiobuttion form control in life mode. The used window
                // is a transparence vcl childwindow. This flag only allows the parent window to
                // paint into the child windows area, but there is no mechanism which takes
                // care for a repaint of the child window. A transparent child window is NOT
                // a window which always keeps it's content consistent over the parent, but it's
                // more like just a paint flag for the parent.
                // To get the update, the windows in question are updated manulally here.
                if(bTargetIsWindow)
                {
                    Window& rWindow = static_cast< Window& >(rmOutputDevice);

                    if(rWindow.IsChildTransparentModeEnabled() && rWindow.GetChildCount())
                    {
                        const Rectangle aRegionRectanglePixel(
                            maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                            maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY());

                        for(sal_uInt16 a(0); a < rWindow.GetChildCount(); a++)
                        {
                            Window* pCandidate = rWindow.GetChild(a);

                            if(pCandidate && pCandidate->IsPaintTransparent())
                            {
                                const Rectangle aCandidatePosSizePixel(pCandidate->GetPosPixel(), pCandidate->GetSizePixel());

                                if(aCandidatePosSizePixel.IsOver(aRegionRectanglePixel))
                                {
                                    pCandidate->Invalidate(INVALIDATE_NOTRANSPARENT|INVALIDATE_CHILDREN);
                                    pCandidate->Update();
                                }
                            }
                        }
                    }
                }

                // #i80730# restore visibility of VCL cursor
                if(bCursorWasEnabled)
                {
                    Window& rWindow = static_cast< Window& >(rmOutputDevice);
                    Cursor* pCursor = rWindow.GetCursor();

                    if(pCursor)
                    {
                        // check if cursor still exists. It may have been deleted from someone
                        pCursor->Show();
                    }
                }

                // forget remembered Region
                maBufferRememberedRangePixel.reset();
            }

            return 0;
        }

        OverlayManagerBuffered::OverlayManagerBuffered(
            OutputDevice& rOutputDevice,
            OverlayManager* pOldOverlayManager,
            bool bRefreshWithPreRendering)
        :   OverlayManager(rOutputDevice, pOldOverlayManager),
            mbRefreshWithPreRendering(bRefreshWithPreRendering)
        {
            // Init timer
            maBufferTimer.SetTimeout(1);
            maBufferTimer.SetTimeoutHdl(LINK(this, OverlayManagerBuffered, ImpBufferTimerHandler));
        }

        rtl::Reference<OverlayManager> OverlayManagerBuffered::create(
            OutputDevice& rOutputDevice,
            OverlayManager* pOldOverlayManager,
            bool bRefreshWithPreRendering)
        {
            return rtl::Reference<OverlayManager>(new OverlayManagerBuffered(rOutputDevice,
                pOldOverlayManager, bRefreshWithPreRendering));
        }

        OverlayManagerBuffered::~OverlayManagerBuffered()
        {
            // Clear timer
            maBufferTimer.Stop();

            if(!maBufferRememberedRangePixel.isEmpty())
            {
                // Restore all rectangles for remembered region from buffer
                ImpRestoreBackground();
            }
        }

        void OverlayManagerBuffered::completeRedraw(const Region& rRegion, OutputDevice* pPreRenderDevice) const
        {
            if(!rRegion.IsEmpty())
            {
                // save new background
                ((OverlayManagerBuffered*)this)->ImpSaveBackground(rRegion, pPreRenderDevice);
            }

            // call parent
            OverlayManager::completeRedraw(rRegion, pPreRenderDevice);
        }

        void OverlayManagerBuffered::flush()
        {
            // call timer handler direct
            ImpBufferTimerHandler(0);
        }

        // #i68597# part of content gets copied, react on it
        void OverlayManagerBuffered::copyArea(const Point& rDestPt, const Point& rSrcPt, const Size& rSrcSize)
        {
            // scroll local buffered area
            maBufferDevice.CopyArea(rDestPt, rSrcPt, rSrcSize);
        }

        void OverlayManagerBuffered::restoreBackground(const Region& rRegion) const
        {
            // restore
            const Region aRegionPixel(getOutputDevice().LogicToPixel(rRegion));
            ImpRestoreBackground(aRegionPixel);

            // call parent
            OverlayManager::restoreBackground(rRegion);
        }

        void OverlayManagerBuffered::invalidateRange(const basegfx::B2DRange& rRange)
        {
            if(!rRange.isEmpty())
            {
                // buffered output, do not invalidate but use the timer
                // to trigger a timer event for refresh
                maBufferTimer.Start();

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
                        (sal_Int32)floor(aDiscreteRange.getMinX() - fDiscreteOne),
                        (sal_Int32)floor(aDiscreteRange.getMinY() - fDiscreteOne));
                    const basegfx::B2IPoint aBottomRight(
                        (sal_Int32)ceil(aDiscreteRange.getMaxX() + fDiscreteOne),
                        (sal_Int32)ceil(aDiscreteRange.getMaxY() + fDiscreteOne));

                    maBufferRememberedRangePixel.expand(aTopLeft);
                    maBufferRememberedRangePixel.expand(aBottomRight);
                }
                else
                {
                    const basegfx::B2IPoint aTopLeft((sal_Int32)floor(aDiscreteRange.getMinX()), (sal_Int32)floor(aDiscreteRange.getMinY()));
                    const basegfx::B2IPoint aBottomRight((sal_Int32)ceil(aDiscreteRange.getMaxX()), (sal_Int32)ceil(aDiscreteRange.getMaxY()));

                    maBufferRememberedRangePixel.expand(aTopLeft);
                    maBufferRememberedRangePixel.expand(aBottomRight);
                }
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
