/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrpaintwindow.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 12:11:39 $
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

#ifndef _SDRPAINTWINDOW_HXX
#include <sdrpaintwindow.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYMANAGERBUFFERED_HXX
#include <svx/sdr/overlay/overlaymanagerbuffered.hxx>
#endif

#ifndef _SVDPNTV_HXX
#include <svdpntv.hxx>
#endif

#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

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
    // expand the ClipRegion if it's a window and we are in paint
    if(!(rExpandedRegion.IsEmpty() || rExpandedRegion.IsNull())
        && OUTDEV_WINDOW == mrOutputDevice.GetOutDevType())
    {
        ((Window&)mrOutputDevice).ExpandPaintClipRegion(rExpandedRegion);
    }

    // calculate the to-be-refreshed rectangle
    Rectangle aPaintRect(rExpandedRegion.GetBoundRect());
    Rectangle aPaintRectPixel = mrOutputDevice.LogicToPixel(aPaintRect);

    // paint using prepared, pre-rendered VirtualDevice
    sal_Bool bMapModeWasEnabledDest(mrOutputDevice.IsMapModeEnabled());
    sal_Bool bMapModeWasEnabledSource(maPreRenderDevice.IsMapModeEnabled());
    mrOutputDevice.EnableMapMode(sal_False);
    maPreRenderDevice.EnableMapMode(sal_False);

    Size aPaintSizePixel = aPaintRectPixel.GetSize();
    mrOutputDevice.DrawOutDev(
        aPaintRectPixel.TopLeft(), aPaintSizePixel,
        aPaintRectPixel.TopLeft(), aPaintSizePixel,
        maPreRenderDevice);

    mrOutputDevice.EnableMapMode(bMapModeWasEnabledDest);
    maPreRenderDevice.EnableMapMode(bMapModeWasEnabledSource);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintWindow::impCreateOverlayManager()
{
    // not yet one created?
    if(!mpOverlayManager)
    {
        // is it a window?
        if(OUTDEV_WINDOW == GetOutputDevice().GetOutDevType())
        {
            // decide which OverlayManager to use
            if(GetPaintView().IsBufferedOverlayAllowed())
            {
                // buffered OverlayManager, buffers it's background and refreshes from there
                // for pure overlay changes (no system redraw). The 2nd parameter specifies
                // if that refresh itself will use a 2nd vdev to avoid flickering.
                mpOverlayManager = new ::sdr::overlay::OverlayManagerBuffered(GetOutputDevice(), sal_True);
            }
            else
            {
                // unbuffered OverlayManager, just invalidates places where changes
                // take place
                mpOverlayManager = new ::sdr::overlay::OverlayManager(GetOutputDevice());
            }

            OSL_ENSURE(mpOverlayManager, "SdrPaintWindow::SdrPaintWindow: Could not allocate an overlayManager (!)");

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
}

SdrPaintWindow::SdrPaintWindow(SdrPaintView& rNewPaintView, OutputDevice& rOut)
:   mrOutputDevice(rOut),
    mrPaintView(rNewPaintView),
    mpOverlayManager(0L),
    mpPreRenderDevice(0L)
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
        const_cast< SdrPaintWindow* >(this)->impCreateOverlayManager();
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

void SdrPaintWindow::DrawOverlay(const Region& rRegion)
{
    // ## force creation of OverlayManager since the first repaint needs to
    // save the background to get a controlled start into overlay mechanism
    impCreateOverlayManager();

    if(mpOverlayManager && !OutputToPrinter())
    {
        if(mpPreRenderDevice)
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
