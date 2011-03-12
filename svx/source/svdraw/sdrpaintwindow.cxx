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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
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

void SdrPaintWindow::impCreateOverlayManager(const bool bUseBuffer)
{
    // When the buffer usage has changed then we have to create a new
    // overlay manager.  Save the current one so that later we can move its
    // overlay objects to the new one.
    sdr::overlay::OverlayManager* pOldOverlayManager = NULL;

    if(mbUseBuffer != bUseBuffer)
    {
        mbUseBuffer = bUseBuffer;
        pOldOverlayManager = mpOverlayManager;
        mpOverlayManager = NULL;
    }

    // not yet one created?
    if(!mpOverlayManager)
    {
        // is it a window?
        if(OUTDEV_WINDOW == GetOutputDevice().GetOutDevType())
        {
            // decide which OverlayManager to use
            if(GetPaintView().IsBufferedOverlayAllowed() && mbUseBuffer)
            {
                // buffered OverlayManager, buffers it's background and refreshes from there
                // for pure overlay changes (no system redraw). The 3rd parameter specifies
                // if that refresh itself will use a 2nd vdev to avoid flickering.
                // Also hand over the evtl. existing old OverlayManager; this means to take over
                // the registered OverlayObjects from it
                mpOverlayManager = new ::sdr::overlay::OverlayManagerBuffered(GetOutputDevice(), pOldOverlayManager, true);
            }
            else
            {
                // unbuffered OverlayManager, just invalidates places where changes
                // take place
                // Also hand over the evtl. existing old OverlayManager; this means to take over
                // the registered OverlayObjects from it
                mpOverlayManager = new ::sdr::overlay::OverlayManager(GetOutputDevice(), pOldOverlayManager);
            }

            OSL_ENSURE(mpOverlayManager, "SdrPaintWindow::SdrPaintWindow: Could not allocate an overlayManager (!)");

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

            mpOverlayManager->setStripeColorA(aColA);
            mpOverlayManager->setStripeColorB(aColB);
            mpOverlayManager->setStripeLengthPixel(GetPaintView().getOptionsDrawinglayer().GetStripeLength());
        }
    }

    // OverlayObjects are transfered for the evtl. newly created OverlayManager by handing over
    // at construction time
    if(pOldOverlayManager)
    {
        // The old overlay manager is not used anymore and can be (has to be) deleted.
        delete pOldOverlayManager;
    }
}

SdrPaintWindow::SdrPaintWindow(SdrPaintView& rNewPaintView, OutputDevice& rOut)
:   mrOutputDevice(rOut),
    mrPaintView(rNewPaintView),
    mpOverlayManager(0L),
    mpPreRenderDevice(0L),
    mbTemporaryTarget(false), // #i72889#
    mbUseBuffer(true)
{
}

SdrPaintWindow::~SdrPaintWindow()
{
    if(mpOverlayManager)
    {
        delete mpOverlayManager;
        mpOverlayManager = 0L;
    }

    DestroyPreRenderDevice();
}

::sdr::overlay::OverlayManager* SdrPaintWindow::GetOverlayManager() const
{
    if(!mpOverlayManager)
    {
        // Create buffered overlay manager by default.
        const_cast< SdrPaintWindow* >(this)->impCreateOverlayManager(true);
    }

    return mpOverlayManager;
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
void SdrPaintWindow::DrawOverlay(const Region& rRegion, bool bUseBuffer)
{
    // ## force creation of OverlayManager since the first repaint needs to
    // save the background to get a controlled start into overlay mechanism
    impCreateOverlayManager(bUseBuffer);

    if(mpOverlayManager && !OutputToPrinter())
    {
        if(mpPreRenderDevice && bUseBuffer)
        {
            mpOverlayManager->completeRedraw(rRegion, &mpPreRenderDevice->GetPreRenderDevice());
        }
        else
        {
            mpOverlayManager->completeRedraw(rRegion);
        }
    }
}

void SdrPaintWindow::HideOverlay(const Region& rRegion)
{
    if(mpOverlayManager && !OutputToPrinter())
    {
        if(!mpPreRenderDevice)
        {
            mpOverlayManager->restoreBackground(rRegion);
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
