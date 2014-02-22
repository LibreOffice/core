/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/sdr/overlay/overlaymanagerbuffered.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/cursor.hxx>
#include <vcl/dibtools.hxx>



namespace sdr
{
    namespace overlay
    {
        void OverlayManagerBuffered::ImpPrepareBufferDevice()
        {
            
            if(maBufferDevice.GetOutputSizePixel() != getOutputDevice().GetOutputSizePixel())
            {
                
                
                maBufferDevice.SetOutputSizePixel(getOutputDevice().GetOutputSizePixel(), false);
            }

            
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
                        
                        const Point aOriginOldPixel(maBufferDevice.LogicToPixel(rOriginOld));
                        const Point aOriginNewPixel(maBufferDevice.LogicToPixel(rOriginNew));
                        const Size aOutputSizePixel(maBufferDevice.GetOutputSizePixel());

                        
                        const bool bMapModeWasEnabled(maBufferDevice.IsMapModeEnabled());
                        maBufferDevice.EnableMapMode(false);

                        
                        const Point aDestinationOffsetPixel(aOriginNewPixel - aOriginOldPixel);
                        maBufferDevice.DrawOutDev(
                            aDestinationOffsetPixel, aOutputSizePixel, 
                            Point(), aOutputSizePixel); 

                        
                        maBufferDevice.EnableMapMode(bMapModeWasEnabled);

                        
                        if(!maBufferRememberedRangePixel.isEmpty())
                        {
                            const basegfx::B2IPoint aIPointDestinationOffsetPixel(aDestinationOffsetPixel.X(), aDestinationOffsetPixel.Y());
                            const basegfx::B2IPoint aNewMinimum(maBufferRememberedRangePixel.getMinimum() + aIPointDestinationOffsetPixel);
                            const basegfx::B2IPoint aNewMaximum(maBufferRememberedRangePixel.getMaximum() + aIPointDestinationOffsetPixel);
                            maBufferRememberedRangePixel = basegfx::B2IRange(aNewMinimum, aNewMaximum);
                        }
                    }
                }

                
                maBufferDevice.SetMapMode(getOutputDevice().GetMapMode());
            }

            
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
            
            const bool bMapModeWasEnabledDest(getOutputDevice().IsMapModeEnabled());
            const bool bMapModeWasEnabledSource(maBufferDevice.IsMapModeEnabled());
            getOutputDevice().EnableMapMode(false);
            ((OverlayManagerBuffered*)this)->maBufferDevice.EnableMapMode(false);

            
            RectangleVector aRectangles;
            rRegionPixel.GetRegionRectangles(aRectangles);

            for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
            {
#ifdef DBG_UTIL
                
                static bool bDoPaintForVisualControl(false);

                if(bDoPaintForVisualControl)
                {
                    getOutputDevice().SetLineColor(COL_LIGHTGREEN);
                    getOutputDevice().SetFillColor();
                    getOutputDevice().DrawRect(*aRectIter);
                }
#endif

                
                const Point aTopLeft(aRectIter->TopLeft());
                const Size aSize(aRectIter->GetSize());

                getOutputDevice().DrawOutDev(
                    aTopLeft, aSize, 
                    aTopLeft, aSize, 
                    maBufferDevice);
            }

            
            
            
            //
            
            
#ifdef DBG_U
            
            
            
            
            
            
            
            
#endif      //
            
            
            
            //
            
            
            
            
            
            //
            

            
            getOutputDevice().EnableMapMode(bMapModeWasEnabledDest);
            ((OverlayManagerBuffered*)this)->maBufferDevice.EnableMapMode(bMapModeWasEnabledSource);
        }

        void OverlayManagerBuffered::ImpSaveBackground(const Region& rRegion, OutputDevice* pPreRenderDevice)
        {
            
            OutputDevice& rSource = (pPreRenderDevice) ? *pPreRenderDevice : getOutputDevice();

            
            ImpPrepareBufferDevice();

            
            Region aRegion(rSource.LogicToPixel(rRegion));

            
            
            if(OUTDEV_WINDOW == rSource.GetOutDevType())
            {
                Window& rWindow = (Window&)rSource;
                Region aPaintRegionPixel = rWindow.LogicToPixel(rWindow.GetPaintRegion());
                aRegion.Intersect(aPaintRegionPixel);

                
                
                rWindow.Flush();
            }

            
            const Rectangle aBufferDeviceRectanglePixel(Point(), maBufferDevice.GetOutputSizePixel());
            aRegion.Intersect(aBufferDeviceRectanglePixel);

            
            const bool bMapModeWasEnabledDest(rSource.IsMapModeEnabled());
            const bool bMapModeWasEnabledSource(maBufferDevice.IsMapModeEnabled());
            rSource.EnableMapMode(false);
            maBufferDevice.EnableMapMode(false);

            
            RectangleVector aRectangles;
            aRegion.GetRegionRectangles(aRectangles);

            for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
            {
                
                const Point aTopLeft(aRectIter->TopLeft());
                const Size aSize(aRectIter->GetSize());

                maBufferDevice.DrawOutDev(
                    aTopLeft, aSize, 
                    aTopLeft, aSize, 
                    rSource);
            }

            
            
            //
            
            
            
            
            
            //
            
            
            
            
            //
            
            //
            

            
            rSource.EnableMapMode(bMapModeWasEnabledDest);
            maBufferDevice.EnableMapMode(bMapModeWasEnabledSource);
        }

        IMPL_LINK(OverlayManagerBuffered, ImpBufferTimerHandler, AutoTimer*, /*pTimer*/)
        {
            
            rtl::Reference<OverlayManager> xRef(this);

            
            maBufferTimer.Stop();

            if(!maBufferRememberedRangePixel.isEmpty())
            {
                
                basegfx::B2DRange aBufferRememberedRangeLogic(
                    maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                    maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY());
                aBufferRememberedRangeLogic.transform(getOutputDevice().GetInverseViewTransformation());

                
                const bool bTargetIsWindow(OUTDEV_WINDOW == rmOutputDevice.GetOutDevType());
                bool bCursorWasEnabled(false);

                
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

                    
                    Rectangle aRegionRectanglePixel(
                        maBufferRememberedRangePixel.getMinX(), maBufferRememberedRangePixel.getMinY(),
                        maBufferRememberedRangePixel.getMaxX(), maBufferRememberedRangePixel.getMaxY());

                    
                    
                    
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

                    
                    const Point aTopLeft(aRegionRectanglePixel.TopLeft());
                    const Size aSize(aRegionRectanglePixel.GetSize());

                    {
                        const bool bMapModeWasEnabledDest(maBufferDevice.IsMapModeEnabled());
                        maBufferDevice.EnableMapMode(false);

                        maOutputBufferDevice.DrawOutDev(
                            aTopLeft, aSize, 
                            aTopLeft, aSize, 
                            maBufferDevice);

                        
                        maBufferDevice.EnableMapMode(bMapModeWasEnabledDest);
                    }

                    
                    
                    maOutputBufferDevice.EnableMapMode(true);
                    OverlayManager::ImpDrawMembers(aBufferRememberedRangeLogic, maOutputBufferDevice);
                    maOutputBufferDevice.EnableMapMode(false);

                    
                    {
                        const bool bMapModeWasEnabledDest(getOutputDevice().IsMapModeEnabled());
                        getOutputDevice().EnableMapMode(false);

                        getOutputDevice().DrawOutDev(
                            aTopLeft, aSize, 
                            aTopLeft, aSize, 
                            maOutputBufferDevice);

                        
                        /*getOutputDevice().SetLineColor(COL_RED);
                        getOutputDevice().SetFillColor();
                        getOutputDevice().DrawRect(Rectangle(aTopLeft, aSize));*/

                        
                        getOutputDevice().EnableMapMode(bMapModeWasEnabledDest);
                    }
                }
                else
                {
                    
                    ImpRestoreBackground();

                    
                    
                    OverlayManager::ImpDrawMembers(aBufferRememberedRangeLogic, getOutputDevice());
                }

                
                
                
                
                
                
                
                
                if(bTargetIsWindow)
                {
                    Window& rWindow = static_cast< Window& >(rmOutputDevice);

                    const Rectangle aRegionRectanglePixel(
                        maBufferRememberedRangePixel.getMinX(),
                        maBufferRememberedRangePixel.getMinY(),
                        maBufferRememberedRangePixel.getMaxX(),
                        maBufferRememberedRangePixel.getMaxY());
                    PaintTransparentChildren(rWindow, aRegionRectanglePixel);
                }

                
                if(bCursorWasEnabled)
                {
                    Window& rWindow = static_cast< Window& >(rmOutputDevice);
                    Cursor* pCursor = rWindow.GetCursor();

                    if(pCursor)
                    {
                        
                        pCursor->Show();
                    }
                }

                
                maBufferRememberedRangePixel.reset();
            }

            return 0;
        }

        OverlayManagerBuffered::OverlayManagerBuffered(
            OutputDevice& rOutputDevice,
            bool bRefreshWithPreRendering)
        :   OverlayManager(rOutputDevice),
            mbRefreshWithPreRendering(bRefreshWithPreRendering)
        {
            
            maBufferTimer.SetTimeout(1);
            maBufferTimer.SetTimeoutHdl(LINK(this, OverlayManagerBuffered, ImpBufferTimerHandler));
        }

        rtl::Reference<OverlayManager> OverlayManagerBuffered::create(
            OutputDevice& rOutputDevice,
            bool bRefreshWithPreRendering)
        {
            return rtl::Reference<OverlayManager>(new OverlayManagerBuffered(rOutputDevice,
                bRefreshWithPreRendering));
        }

        OverlayManagerBuffered::~OverlayManagerBuffered()
        {
            
            maBufferTimer.Stop();

            if(!maBufferRememberedRangePixel.isEmpty())
            {
                
                ImpRestoreBackground();
            }
        }

        void OverlayManagerBuffered::completeRedraw(const Region& rRegion, OutputDevice* pPreRenderDevice) const
        {
            if(!rRegion.IsEmpty())
            {
                
                ((OverlayManagerBuffered*)this)->ImpSaveBackground(rRegion, pPreRenderDevice);
            }

            
            OverlayManager::completeRedraw(rRegion, pPreRenderDevice);
        }

        void OverlayManagerBuffered::flush()
        {
            
            ImpBufferTimerHandler(0);
        }

        
        void OverlayManagerBuffered::copyArea(const Point& rDestPt, const Point& rSrcPt, const Size& rSrcSize)
        {
            
            maBufferDevice.CopyArea(rDestPt, rSrcPt, rSrcSize);
        }

        void OverlayManagerBuffered::restoreBackground(const Region& rRegion) const
        {
            
            const Region aRegionPixel(getOutputDevice().LogicToPixel(rRegion));
            ImpRestoreBackground(aRegionPixel);

            
            OverlayManager::restoreBackground(rRegion);
        }

        void OverlayManagerBuffered::invalidateRange(const basegfx::B2DRange& rRange)
        {
            if(!rRange.isEmpty())
            {
                
                
                maBufferTimer.Start();

                
                
                
                
                
                
                basegfx::B2DRange aDiscreteRange(rRange);
                aDiscreteRange.transform(getOutputDevice().GetViewTransformation());

                if(maDrawinglayerOpt.IsAntiAliasing())
                {
                    
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
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
