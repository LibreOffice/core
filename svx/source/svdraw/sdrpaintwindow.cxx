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

#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/overlay/overlaymanagerbuffered.hxx>
#include <svx/svdpntv.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/svapp.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrPreRenderDevice::SdrPreRenderDevice(OutputDevice& rOriginal)
:   mrOutputDevice(rOriginal)
{
}

SdrPreRenderDevice::~SdrPreRenderDevice()
{
}

void SdrPreRenderDevice::PreparePreRenderDevice()
{
    // compare size of maPreRenderDevice with size of visible area
    if(maPreRenderDevice.GetOutputSizePixel() != mrOutputDevice.GetOutputSizePixel())
    {
        maPreRenderDevice.SetOutputSizePixel(mrOutputDevice.GetOutputSizePixel());
    }

    // Also compare the MapModes for zoom/scroll changes
    if(maPreRenderDevice.GetMapMode() != mrOutputDevice.GetMapMode())
    {
        maPreRenderDevice.SetMapMode(mrOutputDevice.GetMapMode());
    }

    // #i29186#
    maPreRenderDevice.SetDrawMode(mrOutputDevice.GetDrawMode());
    maPreRenderDevice.SetSettings(mrOutputDevice.GetSettings());
}

void SdrPreRenderDevice::OutputPreRenderDevice(const Region& rExpandedRegion)
{
    // region to pixels
    Region aRegionPixel(mrOutputDevice.LogicToPixel(rExpandedRegion));
    RegionHandle aRegionHandle(aRegionPixel.BeginEnumRects());
    Rectangle aRegionRectanglePixel;

    // MapModes off
    sal_Bool bMapModeWasEnabledDest(mrOutputDevice.IsMapModeEnabled());
    sal_Bool bMapModeWasEnabledSource(maPreRenderDevice.IsMapModeEnabled());
    mrOutputDevice.EnableMapMode(sal_False);
    maPreRenderDevice.EnableMapMode(sal_False);

    while(aRegionPixel.GetEnumRects(aRegionHandle, aRegionRectanglePixel))
    {
        // for each rectangle, copy the area
        const Point aTopLeft(aRegionRectanglePixel.TopLeft());
        const Size aSize(aRegionRectanglePixel.GetSize());

        mrOutputDevice.DrawOutDev(
            aTopLeft, aSize,
            aTopLeft, aSize,
            maPreRenderDevice);

#ifdef DBG_UTIL
        // #i74769#
        static bool bDoPaintForVisualControlRegion(false);
        if(bDoPaintForVisualControlRegion)
        {
            Color aColor((((((rand()&0x7f)|0x80)<<8L)|((rand()&0x7f)|0x80))<<8L)|((rand()&0x7f)|0x80));
            mrOutputDevice.SetLineColor(aColor);
            mrOutputDevice.SetFillColor();
            mrOutputDevice.DrawRect(aRegionRectanglePixel);
        }
#endif
    }

    aRegionPixel.EndEnumRects(aRegionHandle);

    mrOutputDevice.EnableMapMode(bMapModeWasEnabledDest);
    maPreRenderDevice.EnableMapMode(bMapModeWasEnabledSource);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintWindow::impCreateOverlayManager()
{
    // not yet one created?
    if(!mxOverlayManager.is())
    {
        // is it a window?
        if(OUTDEV_WINDOW == GetOutputDevice().GetOutDevType())
        {
            // decide which OverlayManager to use
            if(GetPaintView().IsBufferedOverlayAllowed() && mbUseBuffer)
            {
                // buffered OverlayManager, buffers its background and refreshes from there
                // for pure overlay changes (no system redraw). The 3rd parameter specifies
                // whether that refresh itself will use a 2nd vdev to avoid flickering.
                // Also hand over the old OverlayManager if existent; this means to take over
                // the registered OverlayObjects from it
                mxOverlayManager = ::sdr::overlay::OverlayManagerBuffered::create(GetOutputDevice(), true);
            }
            else
            {
                // unbuffered OverlayManager, just invalidates places where changes
                // take place
                // Also hand over the old OverlayManager if existent; this means to take over
                // the registered OverlayObjects from it
                mxOverlayManager = ::sdr::overlay::OverlayManager::create(GetOutputDevice());
            }

            OSL_ENSURE(mxOverlayManager.is(), "SdrPaintWindow::SdrPaintWindow: Could not allocate an overlayManager (!)");

            // Request a repaint so that the buffered overlay manager fills
            // its buffer properly.  This is a workaround for missing buffer
            // updates.
            Window* pWindow = dynamic_cast<Window*>(&GetOutputDevice());
            if (pWindow != NULL)
                pWindow->Invalidate();

            Color aColA(GetPaintView().getOptionsDrawinglayer().GetStripeColorA());
            Color aColB(GetPaintView().getOptionsDrawinglayer().GetStripeColorB());

            if(Application::GetSettings().GetStyleSettings().GetHighContrastMode())
            {
                aColA = aColB = Application::GetSettings().GetStyleSettings().GetHighlightColor();
                aColB.Invert();
            }

            mxOverlayManager->setStripeColorA(aColA);
            mxOverlayManager->setStripeColorB(aColB);
            mxOverlayManager->setStripeLengthPixel(GetPaintView().getOptionsDrawinglayer().GetStripeLength());
        }
    }
}

SdrPaintWindow::SdrPaintWindow(SdrPaintView& rNewPaintView, OutputDevice& rOut)
:   mrOutputDevice(rOut),
    mrPaintView(rNewPaintView),
    mpPreRenderDevice(0L),
    mbTemporaryTarget(false), // #i72889#
    mbUseBuffer(true)
{
}

SdrPaintWindow::~SdrPaintWindow()
{
    mxOverlayManager.clear();

    DestroyPreRenderDevice();
}

rtl::Reference< ::sdr::overlay::OverlayManager > SdrPaintWindow::GetOverlayManager() const
{
    if(!mxOverlayManager.is())
    {
        // Create buffered overlay manager by default.
        const_cast< SdrPaintWindow* >(this)->impCreateOverlayManager();
    }

    return mxOverlayManager;
}

Rectangle SdrPaintWindow::GetVisibleArea() const
{
    Size aVisSizePixel(GetOutputDevice().GetOutputSizePixel());
    return Rectangle(GetOutputDevice().PixelToLogic(Rectangle(Point(0,0), aVisSizePixel)));
}

sal_Bool SdrPaintWindow::OutputToRecordingMetaFile() const
{
    GDIMetaFile* pMetaFile = mrOutputDevice.GetConnectMetaFile();
    return (pMetaFile && pMetaFile->IsRecord() && !pMetaFile->IsPause());
}

void SdrPaintWindow::PreparePreRenderDevice()
{
    const sal_Bool bPrepareBufferedOutput(
        mrPaintView.IsBufferedOutputAllowed()
        && !OutputToPrinter()
        && !OutputToVirtualDevice()
        && !OutputToRecordingMetaFile());

    if(bPrepareBufferedOutput)
    {
        if(!mpPreRenderDevice)
        {
            mpPreRenderDevice = new SdrPreRenderDevice(mrOutputDevice);
        }
    }
    else
    {
        DestroyPreRenderDevice();
    }

    if(mpPreRenderDevice)
    {
        mpPreRenderDevice->PreparePreRenderDevice();
    }
}

void SdrPaintWindow::DestroyPreRenderDevice()
{
    if(mpPreRenderDevice)
    {
        delete mpPreRenderDevice;
        mpPreRenderDevice = 0L;
    }
}

void SdrPaintWindow::OutputPreRenderDevice(const Region& rExpandedRegion)
{
    if(mpPreRenderDevice)
    {
        mpPreRenderDevice->OutputPreRenderDevice(rExpandedRegion);
    }
}

// #i73602# add flag if buffer shall be used
void SdrPaintWindow::DrawOverlay(const Region& rRegion)
{
    // ## force creation of OverlayManager since the first repaint needs to
    // save the background to get a controlled start into overlay mechanism
    impCreateOverlayManager();

    if(mxOverlayManager.is() && !OutputToPrinter())
    {
        if(mpPreRenderDevice)
        {
            mxOverlayManager->completeRedraw(rRegion, &mpPreRenderDevice->GetPreRenderDevice());
        }
        else
        {
            mxOverlayManager->completeRedraw(rRegion);
        }
    }
}

const Region& SdrPaintWindow::GetRedrawRegion() const
{
    return maRedrawRegion;
}

void SdrPaintWindow::SetRedrawRegion(const Region& rNew)
{
    maRedrawRegion = rNew;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
