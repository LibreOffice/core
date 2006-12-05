/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlaymanagerbuffered.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 12:11:20 $
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
#include "precompiled_svx.hxx"

#ifndef _SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX
#include <svx/sdr/overlay/overlaymanagerbuffered.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

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
                const sal_Bool bZoomed(
                    maBufferDevice.GetMapMode().GetScaleX() != getOutputDevice().GetMapMode().GetScaleX()
                    || maBufferDevice.GetMapMode().GetScaleY() != getOutputDevice().GetMapMode().GetScaleY());

                if(!bZoomed)
                {
                    const Point& rOriginOld = maBufferDevice.GetMapMode().GetOrigin();
                    const Point& rOriginNew = getOutputDevice().GetMapMode().GetOrigin();
                    const sal_Bool bScrolled(rOriginOld != rOriginNew);

                    if(bScrolled)
                    {
                        // get pixel bounds
                        const Point aOriginOldPixel(maBufferDevice.LogicToPixel(rOriginOld));
                        const Point aOriginNewPixel(maBufferDevice.LogicToPixel(rOriginNew));
                        const Size aOutputSizePixel(maBufferDevice.GetOutputSizePixel());

                        // remember and switch off MapMode
                        const sal_Bool bMapModeWasEnabled(maBufferDevice.IsMapModeEnabled());
                        maBufferDevice.EnableMapMode(sal_False);

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
        }

        void OverlayManagerBuffered::ImpRestoreBackground() const
        {
            const Rectangle aRegionRectanglePixel(
                maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY());
            ImpRestoreBackground(aRegionRectanglePixel);
        }

        void OverlayManagerBuffered::ImpRestoreBackground(const Rectangle& rRegionRectanglePixel) const
        {
            // MapModes off
            const sal_Bool bMapModeWasEnabledDest(getOutputDevice().IsMapModeEnabled());
            const sal_Bool bMapModeWasEnabledSource(maBufferDevice.IsMapModeEnabled());
            getOutputDevice().EnableMapMode(sal_False);
            ((OverlayManagerBuffered*)this)->maBufferDevice.EnableMapMode(sal_False);

            // restore the area
            const Point aTopLeft(rRegionRectanglePixel.TopLeft());
            const Size aSize(rRegionRectanglePixel.GetSize());

            getOutputDevice().DrawOutDev(
                aTopLeft, aSize, // destination
                aTopLeft, aSize, // source
                maBufferDevice);

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
            }

            // also limit to buffer size
            const Rectangle aBufferDeviceRectanglePixel = Rectangle(Point(), maBufferDevice.GetOutputSizePixel());
            aRegion.Intersect(aBufferDeviceRectanglePixel);

            // prepare to iterate over the rectangles from the region in pixels
            RegionHandle aRegionHandle(aRegion.BeginEnumRects());
            Rectangle aRegionRectanglePixel;

            // MapModes off
            const sal_Bool bMapModeWasEnabledDest(rSource.IsMapModeEnabled());
            const sal_Bool bMapModeWasEnabledSource(maBufferDevice.IsMapModeEnabled());
            rSource.EnableMapMode(sal_False);
            maBufferDevice.EnableMapMode(sal_False);

            while(aRegion.GetEnumRects(aRegionHandle, aRegionRectanglePixel))
            {
                // for each rectangle, save the area
                Point aTopLeft(aRegionRectanglePixel.TopLeft());
                Size aSize(aRegionRectanglePixel.GetSize());

                maBufferDevice.DrawOutDev(
                    aTopLeft, aSize, // destination
                    aTopLeft, aSize, // source
                    rSource);

                static bool bDoPaintForVisualControl(false);
                if(bDoPaintForVisualControl)
                {
                    getOutputDevice().SetLineColor(COL_LIGHTRED);
                    getOutputDevice().SetFillColor();
                    getOutputDevice().DrawRect(aRegionRectanglePixel);
                }
            }

            aRegion.EndEnumRects(aRegionHandle);

            // restore MapModes
            rSource.EnableMapMode(bMapModeWasEnabledDest);
            maBufferDevice.EnableMapMode(bMapModeWasEnabledSource);
        }

        IMPL_LINK(OverlayManagerBuffered, ImpBufferTimerHandler, AutoTimer*, /*pTimer*/)
        {
            // stop timer
            maBufferTimer.Stop();

            if(!maBufferRememberedRangePixel.isEmpty())
            {
                // logic size for impDrawMember call
                const Rectangle aRectangleLogic(getOutputDevice().PixelToLogic(Rectangle(
                    maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                    maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY())));
                const basegfx::B2DRange aBufferRememberedRangeLogic(aRectangleLogic.Left(), aRectangleLogic.Top(), aRectangleLogic.Right(), aRectangleLogic.Bottom());

                if(DoRefreshWithPreRendering())
                {
                    // calculate sizes
                    Rectangle aRegionRectanglePixel(
                        maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                        maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY());

                    // truncate aRegionRectanglePixel to destination pixel size, more does
                    // not need to be prepared since destination is a buffer for a window. So,
                    // maximum size indirectly shall be limited to getOutputDevice().GetOutputSizePixel()
                    const Size aDestinationSizePixel(maBufferDevice.GetOutputSizePixel());

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

                    // clone the maBufferDevice and copy content
                    VirtualDevice aOutputBuffer(maBufferDevice, 0);
                    aOutputBuffer.SetOutputSizePixel(aSize);
                    aOutputBuffer.SetMapMode(getOutputDevice().GetMapMode());
                    aOutputBuffer.EnableMapMode(sal_False);

                    // #i29186#
                    aOutputBuffer.SetDrawMode(maBufferDevice.GetDrawMode());
                    aOutputBuffer.SetSettings(maBufferDevice.GetSettings());

                    {
                        const sal_Bool bMapModeWasEnabledDest(maBufferDevice.IsMapModeEnabled());
                        maBufferDevice.EnableMapMode(sal_False);

                        aOutputBuffer.DrawOutDev(
                            Point(), aSize, // destination
                            aTopLeft, aSize, // source
                            maBufferDevice);

                        // restore MapModes
                        maBufferDevice.EnableMapMode(bMapModeWasEnabledDest);
                    }

                    // paint overlay content for remembered region, use
                    // method from base class directly
                    aOutputBuffer.EnableMapMode(sal_True);
                    aOutputBuffer.SetPixelOffset(Size(-aTopLeft.X(), -aTopLeft.Y()));
                    OverlayManager::ImpDrawMembers(aBufferRememberedRangeLogic, aOutputBuffer);
                    aOutputBuffer.SetPixelOffset(Size(0, 0));
                    aOutputBuffer.EnableMapMode(sal_False);

                    // copy to output
                    {
                        const sal_Bool bMapModeWasEnabledDest(getOutputDevice().IsMapModeEnabled());
                        getOutputDevice().EnableMapMode(sal_False);

                        getOutputDevice().DrawOutDev(
                            aTopLeft, aSize, // destination
                            Point(), aSize, // source
                            aOutputBuffer);

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
                if(OUTDEV_WINDOW == rmOutputDevice.GetOutDevType())
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

                // forget remembered Region
                maBufferRememberedRangePixel.reset();
            }

            return 0;
        }

        OverlayManagerBuffered::OverlayManagerBuffered(OutputDevice& rOutputDevice, sal_Bool bRefreshWithPreRendering)
        :   OverlayManager(rOutputDevice),
            mbRefreshWithPreRendering(bRefreshWithPreRendering)
        {
            // Init timer
            maBufferTimer.SetTimeout(1);
            maBufferTimer.SetTimeoutHdl(LINK(this, OverlayManagerBuffered, ImpBufferTimerHandler));
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

        void OverlayManagerBuffered::restoreBackground(const Region& rRegion) const
        {
            // restore
            Rectangle aBoundRect(rRegion.GetBoundRect());
            aBoundRect = getOutputDevice().LogicToPixel(aBoundRect);
            ImpRestoreBackground(aBoundRect);

            // call parent
            OverlayManager::restoreBackground(rRegion);
        }

        void OverlayManagerBuffered::invalidateRange(const basegfx::B2DRange& rRange)
        {
            // buffered output, do not invalidate but use the timer
            // to trigger a timer event for refresh
            maBufferTimer.Start();

            // also add the range to the remembered region
            const Point aTL((sal_Int32)floor(rRange.getMinX()), (sal_Int32)floor(rRange.getMinY()));
            const Point aBR((sal_Int32)floor(rRange.getMaxX()) + 1L, (sal_Int32)floor(rRange.getMaxY()) + 1L);
            const Point aTLPix(getOutputDevice().LogicToPixel(aTL));
            const Point aBRPix(getOutputDevice().LogicToPixel(aBR));

            maBufferRememberedRangePixel.expand(basegfx::B2IPoint(aTLPix.X(), aTLPix.Y()));
            maBufferRememberedRangePixel.expand(basegfx::B2IPoint(aBRPix.X(), aBRPix.Y()));
        }

        void OverlayManagerBuffered::SetRefreshWithPreRendering(sal_Bool bNew)
        {
            if(mbRefreshWithPreRendering != bNew)
            {
                mbRefreshWithPreRendering = bNew;
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
