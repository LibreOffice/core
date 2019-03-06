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

#include <comphelper/lok.hxx>
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
    DECL_LINK(WindowEventListener, VclWindowEvent&, void);
public:
    void PaintTransparentChildren(vcl::Window const & rWindow, tools::Rectangle const& rPixelRect);
    ~CandidateMgr();
};

IMPL_LINK(CandidateMgr, WindowEventListener, VclWindowEvent&, rEvent, void)
{
    vcl::Window* pWindow = rEvent.GetWindow();
    if (rEvent.GetId() == VclEventId::ObjectDying)
    {
        m_aDeletedCandidates.insert(pWindow);
    }
}

CandidateMgr::~CandidateMgr()
{
    for (VclPtr<vcl::Window>& pCandidate : m_aCandidates)
    {
        if (m_aDeletedCandidates.find(pCandidate) != m_aDeletedCandidates.end())
            continue;
        pCandidate->RemoveEventListener(LINK(this, CandidateMgr, WindowEventListener));
    }
}

void PaintTransparentChildren(vcl::Window const & rWindow, tools::Rectangle const& rPixelRect)
{
    if (!rWindow.IsChildTransparentModeEnabled())
        return;

    CandidateMgr aManager;
    aManager.PaintTransparentChildren(rWindow, rPixelRect);
}

void CandidateMgr::PaintTransparentChildren(vcl::Window const & rWindow, tools::Rectangle const& rPixelRect)
{
    vcl::Window * pCandidate = rWindow.GetWindow( GetWindowType::FirstChild );
    while (pCandidate)
    {
        if (pCandidate->IsPaintTransparent())
        {
            const tools::Rectangle aCandidatePosSizePixel(
                            pCandidate->GetPosPixel(),
                            pCandidate->GetSizePixel());

            if (aCandidatePosSizePixel.IsOver(rPixelRect))
            {
                m_aCandidates.emplace_back(pCandidate);
                pCandidate->AddEventListener(LINK(this, CandidateMgr, WindowEventListener));
            }
        }
        pCandidate = pCandidate->GetWindow( GetWindowType::Next );
    }

    for (const auto& rpCandidate : m_aCandidates)
    {
        pCandidate = rpCandidate.get();
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
:   mpOutputDevice(&rOriginal),
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
    if(mpPreRenderDevice->GetOutputSizePixel() != mpOutputDevice->GetOutputSizePixel())
    {
        mpPreRenderDevice->SetOutputSizePixel(mpOutputDevice->GetOutputSizePixel());
    }

    // Also compare the MapModes for zoom/scroll changes
    if(mpPreRenderDevice->GetMapMode() != mpOutputDevice->GetMapMode())
    {
        mpPreRenderDevice->SetMapMode(mpOutputDevice->GetMapMode());
    }

    // #i29186#
    mpPreRenderDevice->SetDrawMode(mpOutputDevice->GetDrawMode());
    mpPreRenderDevice->SetSettings(mpOutputDevice->GetSettings());
}

void SdrPreRenderDevice::OutputPreRenderDevice(const vcl::Region& rExpandedRegion)
{
    // region to pixels
    const vcl::Region aRegionPixel(mpOutputDevice->LogicToPixel(rExpandedRegion));
    //RegionHandle aRegionHandle(aRegionPixel.BeginEnumRects());
    //Rectangle aRegionRectanglePixel;

    // MapModes off
    bool bMapModeWasEnabledDest(mpOutputDevice->IsMapModeEnabled());
    bool bMapModeWasEnabledSource(mpPreRenderDevice->IsMapModeEnabled());
    mpOutputDevice->EnableMapMode(false);
    mpPreRenderDevice->EnableMapMode(false);

    RectangleVector aRectangles;
    aRegionPixel.GetRegionRectangles(aRectangles);

    for(const auto& rRect : aRectangles)
    {
        // for each rectangle, copy the area
        const Point aTopLeft(rRect.TopLeft());
        const Size aSize(rRect.GetSize());

        mpOutputDevice->DrawOutDev(
            aTopLeft, aSize,
            aTopLeft, aSize,
            *mpPreRenderDevice);
    }

    mpOutputDevice->EnableMapMode(bMapModeWasEnabledDest);
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
                mxOverlayManager = sdr::overlay::OverlayManagerBuffered::create(GetOutputDevice());
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
            if (!comphelper::LibreOfficeKit::isActive())
            {
                rWindow.Invalidate();
            }

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
:   mpOutputDevice(&rOut),
    mpWindow(pWindow),
    mrPaintView(rNewPaintView),
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

tools::Rectangle SdrPaintWindow::GetVisibleArea() const
{
    Size aVisSizePixel(GetOutputDevice().GetOutputSizePixel());
    return GetOutputDevice().PixelToLogic(tools::Rectangle(Point(0,0), aVisSizePixel));
}

bool SdrPaintWindow::OutputToRecordingMetaFile() const
{
    GDIMetaFile* pMetaFile = mpOutputDevice->GetConnectMetaFile();
    return (pMetaFile && pMetaFile->IsRecord() && !pMetaFile->IsPause());
}

void SdrPaintWindow::PreparePreRenderDevice()
{
    const bool bPrepareBufferedOutput(
        mrPaintView.IsBufferedOutputAllowed()
        && !OutputToPrinter()
        && !OutputIsVirtualDevice()
        && !OutputToRecordingMetaFile());

    if(bPrepareBufferedOutput)
    {
        if(!mpPreRenderDevice)
        {
            mpPreRenderDevice.reset(new SdrPreRenderDevice(*mpOutputDevice));
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
    mpPreRenderDevice.reset();
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
