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

#include <comphelper/random.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <sdr/overlay/overlaymanagerbuffered.hxx>
#include <svx/svdpntv.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <set>
#include <vector>

//rhbz#1007697 do this in two loops, one to collect the candidates
//and another to update them because updating a candidate can
//trigger the candidate to be deleted, so asking for its
//sibling after that is going to fail hard
class CandidateMgr
{
    std::vector<VclPtr<vcl::Window> > m_aCandidates;
    std::set<VclPtr<vcl::Window> > m_aDeletedCandidates;
    DECL_LINK_TYPED(WindowEventListener, VclWindowEvent&, void);
public:
    void PaintTransparentChildren(vcl::Window & rWindow, Rectangle const& rPixelRect);
    ~CandidateMgr();
};

IMPL_LINK_TYPED(CandidateMgr, WindowEventListener, VclWindowEvent&, rEvent, void)
{
    vcl::Window* pWindow = rEvent.GetWindow();
    if (rEvent.GetId() == VCLEVENT_OBJECT_DYING)
    {
        m_aDeletedCandidates.insert(pWindow);
    }
}

CandidateMgr::~CandidateMgr()
{
    for (auto aI = m_aCandidates.begin(); aI != m_aCandidates.end(); ++aI)
    {
        VclPtr<vcl::Window> pCandidate = *aI;
        if (m_aDeletedCandidates.find(pCandidate) != m_aDeletedCandidates.end())
            continue;
        pCandidate->RemoveEventListener(LINK(this, CandidateMgr, WindowEventListener));
    }
}

void PaintTransparentChildren(vcl::Window & rWindow, Rectangle const& rPixelRect)
{
    if (!rWindow.IsChildTransparentModeEnabled())
        return;

    CandidateMgr aManager;
    aManager.PaintTransparentChildren(rWindow, rPixelRect);
}

void CandidateMgr::PaintTransparentChildren(vcl::Window & rWindow, Rectangle const& rPixelRect)
{
    vcl::Window * pCandidate = rWindow.GetWindow( GetWindowType::FirstChild );
    while (pCandidate)
    {
        if (pCandidate->IsPaintTransparent())
        {
            const Rectangle aCandidatePosSizePixel(
                            pCandidate->GetPosPixel(),
                            pCandidate->GetSizePixel());

            if (aCandidatePosSizePixel.IsOver(rPixelRect))
            {
                m_aCandidates.push_back(pCandidate);
                pCandidate->AddEventListener(LINK(this, CandidateMgr, WindowEventListener));
            }
        }
        pCandidate = pCandidate->GetWindow( GetWindowType::Next );
    }

    for (auto aI = m_aCandidates.begin(); aI != m_aCandidates.end(); ++aI)
    {
        pCandidate = *aI;
        if (m_aDeletedCandidates.find(pCandidate) != m_aDeletedCandidates.end())
            continue;
        //rhbz#1007697 this can cause the window itself to be
        //deleted. So we are listening to see if that happens
        //and if so, then skip the update
        pCandidate->Invalidate(InvalidateFlags::NoTransparent|InvalidateFlags::Children);
        // important: actually paint the child here!
        if (m_aDeletedCandidates.find(pCandidate) != m_aDeletedCandidates.end())
            continue;
        pCandidate->Update();
    }
}

SdrPreRenderDevice::SdrPreRenderDevice(OutputDevice& rOriginal)
:   mrOutputDevice(rOriginal),
    mpPreRenderDevice(VclPtr<VirtualDevice>::Create())
{
}

SdrPreRenderDevice::~SdrPreRenderDevice()
{
    mpPreRenderDevice.disposeAndClear();
}

void SdrPreRenderDevice::PreparePreRenderDevice()
{
    // compare size of mpPreRenderDevice with size of visible area
    if(mpPreRenderDevice->GetOutputSizePixel() != mrOutputDevice.GetOutputSizePixel())
    {
        mpPreRenderDevice->SetOutputSizePixel(mrOutputDevice.GetOutputSizePixel());
    }

    // Also compare the MapModes for zoom/scroll changes
    if(mpPreRenderDevice->GetMapMode() != mrOutputDevice.GetMapMode())
    {
        mpPreRenderDevice->SetMapMode(mrOutputDevice.GetMapMode());
    }

    // #i29186#
    mpPreRenderDevice->SetDrawMode(mrOutputDevice.GetDrawMode());
    mpPreRenderDevice->SetSettings(mrOutputDevice.GetSettings());
}

void SdrPreRenderDevice::OutputPreRenderDevice(const vcl::Region& rExpandedRegion)
{
    // region to pixels
    const vcl::Region aRegionPixel(mrOutputDevice.LogicToPixel(rExpandedRegion));
    //RegionHandle aRegionHandle(aRegionPixel.BeginEnumRects());
    //Rectangle aRegionRectanglePixel;

    // MapModes off
    bool bMapModeWasEnabledDest(mrOutputDevice.IsMapModeEnabled());
    bool bMapModeWasEnabledSource(mpPreRenderDevice->IsMapModeEnabled());
    mrOutputDevice.EnableMapMode(false);
    mpPreRenderDevice->EnableMapMode(false);

    RectangleVector aRectangles;
    aRegionPixel.GetRegionRectangles(aRectangles);

    for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
    {
        // for each rectangle, copy the area
        const Point aTopLeft(aRectIter->TopLeft());
        const Size aSize(aRectIter->GetSize());

        mrOutputDevice.DrawOutDev(
            aTopLeft, aSize,
            aTopLeft, aSize,
            *mpPreRenderDevice.get());

#ifdef DBG_UTIL
        // #i74769#
        static bool bDoPaintForVisualControlRegion(false);

        if(bDoPaintForVisualControlRegion)
        {
            int nR = comphelper::rng::uniform_int_distribution(0, 0x7F-1);
            int nG = comphelper::rng::uniform_int_distribution(0, 0x7F-1);
            int nB = comphelper::rng::uniform_int_distribution(0, 0x7F-1);
            const Color aColor(((((nR|0x80)<<8L)|(nG|0x80))<<8L)|(nB|0x80));

            mrOutputDevice.SetLineColor(aColor);
            mrOutputDevice.SetFillColor();
            mrOutputDevice.DrawRect(*aRectIter);
        }
#endif
    }

    mrOutputDevice.EnableMapMode(bMapModeWasEnabledDest);
    mpPreRenderDevice->EnableMapMode(bMapModeWasEnabledSource);
}


void SdrPaintWindow::impCreateOverlayManager()
{
    // not yet one created?
    if(!mxOverlayManager.is())
    {
        // is it a window?
        if(OUTDEV_WINDOW == GetOutputDevice().GetOutDevType())
        {
            vcl::Window& rWindow = dynamic_cast<vcl::Window&>(GetOutputDevice());
            // decide which OverlayManager to use
            if(GetPaintView().IsBufferedOverlayAllowed() && !rWindow.SupportsDoubleBuffering())
            {
                // buffered OverlayManager, buffers its background and refreshes from there
                // for pure overlay changes (no system redraw). The 3rd parameter specifies
                // whether that refresh itself will use a 2nd vdev to avoid flickering.
                // Also hand over the old OverlayManager if existent; this means to take over
                // the registered OverlayObjects from it
                mxOverlayManager = sdr::overlay::OverlayManagerBuffered::create(GetOutputDevice(), true);
            }
            else
            {
                // unbuffered OverlayManager, just invalidates places where changes
                // take place
                // Also hand over the old OverlayManager if existent; this means to take over
                // the registered OverlayObjects from it
                mxOverlayManager = sdr::overlay::OverlayManager::create(GetOutputDevice());
            }

            OSL_ENSURE(mxOverlayManager.is(), "SdrPaintWindow::SdrPaintWindow: Could not allocate an overlayManager (!)");

            // Request a repaint so that the buffered overlay manager fills
            // its buffer properly.  This is a workaround for missing buffer
            // updates.
            rWindow.Invalidate();

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

SdrPaintWindow::SdrPaintWindow(SdrPaintView& rNewPaintView, OutputDevice& rOut, vcl::Window* pWindow)
:   mrOutputDevice(rOut),
    mpWindow(pWindow),
    mrPaintView(rNewPaintView),
    mpPreRenderDevice(nullptr),
    mbTemporaryTarget(false) // #i72889#
{
}

SdrPaintWindow::~SdrPaintWindow()
{
    mxOverlayManager.clear();

    DestroyPreRenderDevice();
}

rtl::Reference< sdr::overlay::OverlayManager > const & SdrPaintWindow::GetOverlayManager() const
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

bool SdrPaintWindow::OutputToRecordingMetaFile() const
{
    GDIMetaFile* pMetaFile = mrOutputDevice.GetConnectMetaFile();
    return (pMetaFile && pMetaFile->IsRecord() && !pMetaFile->IsPause());
}

void SdrPaintWindow::PreparePreRenderDevice()
{
    const bool bPrepareBufferedOutput(
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
        mpPreRenderDevice = nullptr;
    }
}

void SdrPaintWindow::OutputPreRenderDevice(const vcl::Region& rExpandedRegion)
{
    if(mpPreRenderDevice)
    {
        mpPreRenderDevice->OutputPreRenderDevice(rExpandedRegion);
    }
}

// #i73602# add flag if buffer shall be used
void SdrPaintWindow::DrawOverlay(const vcl::Region& rRegion)
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


void SdrPaintWindow::SetRedrawRegion(const vcl::Region& rNew)
{
    maRedrawRegion = rNew;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
