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

#include <svx/sdrpagewindow.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/fmview.hxx>
#include <sfx2/lokhelper.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <tools/debug.hxx>
#include <vcl/window.hxx>

using namespace ::com::sun::star;

struct SdrPageWindow::Impl
{
    // #110094# ObjectContact section
    mutable sdr::contact::ObjectContact* mpObjectContact;

    // the SdrPageView this window belongs to
    SdrPageView& mrPageView;

    // the PaintWindow to paint on. Here is access to OutDev etc.
    // #i72752# change to pointer to allow patcing it in DrawLayer() if necessary
    SdrPaintWindow* mpPaintWindow;
    SdrPaintWindow* mpOriginalPaintWindow;

    // UNO stuff for xControls
    uno::Reference<awt::XControlContainer> mxControlContainer;

    Impl( SdrPageView& rPageView, SdrPaintWindow& rPaintWindow ) :
        mpObjectContact(nullptr),
        mrPageView(rPageView),
        mpPaintWindow(&rPaintWindow),
        mpOriginalPaintWindow(nullptr)
    {
    }
};


uno::Reference<awt::XControlContainer> const & SdrPageWindow::GetControlContainer( bool _bCreateIfNecessary ) const
{
    if (!mpImpl->mxControlContainer.is() && _bCreateIfNecessary)
    {
        SdrView& rView = GetPageView().GetView();

        const SdrPaintWindow& rPaintWindow( GetOriginalPaintWindow() ? *GetOriginalPaintWindow() : GetPaintWindow() );
        if ( rPaintWindow.OutputToWindow() && !rView.IsPrintPreview() )
        {
            vcl::Window* pWindow = rPaintWindow.GetOutputDevice().GetOwnerWindow();
            const_cast< SdrPageWindow* >( this )->mpImpl->mxControlContainer = VCLUnoHelper::CreateControlContainer( pWindow );

            // #100394# xC->setVisible triggers window->Show() and this has
            // problems when the view is not completely constructed which may
            // happen when loading. This leads to accessibility broadcasts which
            // throw asserts due to the not finished view. All this chain can be avoided
            // since xC->setVisible is here called only for the side effect in
            // UnoControlContainer::setVisible(...) which calls createPeer(...).
            // This will now be called directly from here.

            uno::Reference< awt::XControl > xControl(mpImpl->mxControlContainer, uno::UNO_QUERY);
            if(xControl.is())
            {
                uno::Reference< uno::XInterface > xContext = xControl->getContext();
                if(!xContext.is())
                {
                    xControl->createPeer( uno::Reference<awt::XToolkit>(), uno::Reference<awt::XWindowPeer>() );
                }
            }
        }
        else
        {
            // Printer and VirtualDevice, or rather: no OutDev
            uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            const_cast< SdrPageWindow* >( this )->mpImpl->mxControlContainer.set(xFactory->createInstance("com.sun.star.awt.UnoControlContainer"), uno::UNO_QUERY);
            uno::Reference< awt::XControlModel > xModel(xFactory->createInstance("com.sun.star.awt.UnoControlContainerModel"), uno::UNO_QUERY);
            uno::Reference< awt::XControl > xControl(mpImpl->mxControlContainer, uno::UNO_QUERY);
            if (xControl.is())
                xControl->setModel(xModel);

            OutputDevice& rOutDev = rPaintWindow.GetOutputDevice();
            Point aPosPix = rOutDev.GetMapMode().GetOrigin();
            Size aSizePix = rOutDev.GetOutputSizePixel();

            uno::Reference< awt::XWindow > xContComp(mpImpl->mxControlContainer, uno::UNO_QUERY);
            if( xContComp.is() )
                xContComp->setPosSize(aPosPix.X(), aPosPix.Y(), aSizePix.Width(), aSizePix.Height(), awt::PosSize::POSSIZE);
        }

        FmFormView* pViewAsFormView = dynamic_cast< FmFormView* >( &rView );
        if ( pViewAsFormView )
            pViewAsFormView->InsertControlContainer(mpImpl->mxControlContainer);
    }
    return mpImpl->mxControlContainer;
}

SdrPageWindow::SdrPageWindow(SdrPageView& rPageView, SdrPaintWindow& rPaintWindow) :
    mpImpl(new Impl(rPageView, rPaintWindow))
{
}

SdrPageWindow::~SdrPageWindow()
{
    // #i26631#
    ResetObjectContact();

    if (!mpImpl->mxControlContainer.is())
        return;

    auto & rView = static_cast<SdrPaintView &>(GetPageView().GetView());

    // notify derived views
    FmFormView* pViewAsFormView = dynamic_cast< FmFormView* >( &rView );
    if ( pViewAsFormView )
        pViewAsFormView->RemoveControlContainer(mpImpl->mxControlContainer);

    // dispose the control container
    uno::Reference< lang::XComponent > xComponent(mpImpl->mxControlContainer, uno::UNO_QUERY);
    xComponent->dispose();
}

SdrPageView& SdrPageWindow::GetPageView() const
{
    return mpImpl->mrPageView;
}

SdrPaintWindow& SdrPageWindow::GetPaintWindow() const
{
    return *mpImpl->mpPaintWindow;
}

const SdrPaintWindow* SdrPageWindow::GetOriginalPaintWindow() const
{
    return mpImpl->mpOriginalPaintWindow;
}

// OVERLAY MANAGER
rtl::Reference< sdr::overlay::OverlayManager > const & SdrPageWindow::GetOverlayManager() const
{
    return GetPaintWindow().GetOverlayManager();
}

SdrPaintWindow* SdrPageWindow::patchPaintWindow(SdrPaintWindow& rPaintWindow)
{
    if (!mpImpl)
        return nullptr;

    if (!mpImpl->mpOriginalPaintWindow)
    {
        // first patch
        mpImpl->mpOriginalPaintWindow = mpImpl->mpPaintWindow;
        mpImpl->mpPaintWindow = &rPaintWindow;
        mpImpl->mpOriginalPaintWindow->setPatched(&rPaintWindow);
        return mpImpl->mpOriginalPaintWindow;
    }
    else
    {
        // second or more patch
        auto pPreviousPaintWindow = mpImpl->mpPaintWindow;
        mpImpl->mpPaintWindow = &rPaintWindow;
        mpImpl->mpOriginalPaintWindow->setPatched(&rPaintWindow);
        return pPreviousPaintWindow;
    }
}

void SdrPageWindow::unpatchPaintWindow(SdrPaintWindow* pPreviousPaintWindow)
{
    if (pPreviousPaintWindow == mpImpl->mpOriginalPaintWindow)
    {
        // first patch
        mpImpl->mpPaintWindow = mpImpl->mpOriginalPaintWindow;
        mpImpl->mpOriginalPaintWindow->setPatched(nullptr);
        mpImpl->mpOriginalPaintWindow = nullptr;
    }
    else
    {
        // second or more patch
        mpImpl->mpPaintWindow = pPreviousPaintWindow;
        mpImpl->mpOriginalPaintWindow->setPatched(pPreviousPaintWindow);
    }
}

void SdrPageWindow::PrePaint()
{
    // give OC the chance to do ProcessDisplay preparations
    if(HasObjectContact())
    {
        GetObjectContact().PrepareProcessDisplay();
    }
}

void SdrPageWindow::PrepareRedraw(const vcl::Region& rReg)
{
    // give OC the chance to do ProcessDisplay preparations
    if(HasObjectContact())
    {
        GetObjectContact().PrepareProcessDisplay();
    }

    // if necessary, remember changed RedrawArea at PaintWindow for usage with
    // overlay and PreRenderDevice stuff
    GetPaintWindow().SetRedrawRegion(rReg);
}


// clip test
#ifdef CLIPPER_TEST
#include <svx/svdopath.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <tools/helpers.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>

// for ::std::sort
#include <algorithm>

namespace
{
    void impPaintStrokePolygon(const basegfx::B2DPolygon& rCandidate, OutputDevice& rOutDev, Color aColor)
    {
        basegfx::B2DPolygon aCandidate(rCandidate);

        if(aCandidate.areControlPointsUsed())
        {
            aCandidate = basegfx::utils::adaptiveSubdivideByAngle(rCandidate);
        }

        if(aCandidate.count())
        {
            const sal_uInt32 nLoopCount(aCandidate.isClosed() ? aCandidate.count() : aCandidate.count() - 1);
            rOutDev.SetFillColor();
            rOutDev.SetLineColor(aColor);

            for(sal_uInt32 a(0); a < nLoopCount; a++)
            {
                const basegfx::B2DPoint aBStart(aCandidate.getB2DPoint(a));
                const basegfx::B2DPoint aBEnd(aCandidate.getB2DPoint((a + 1) % aCandidate.count()));
                const Point aStart(basegfx::fround<tools::Long>(aBStart.getX()),
                                   basegfx::fround<tools::Long>(aBStart.getY()));
                const Point aEnd(basegfx::fround<tools::Long>(aBEnd.getX()),
                                 basegfx::fround<tools::Long>(aBEnd.getY()));
                rOutDev.DrawLine(aStart, aEnd);
            }
        }
    }

    void impTryTest(const SdrPageView& rPageView, OutputDevice& rOutDev)
    {
        if(rPageView.GetPage() && rPageView.GetPage()->GetObjCount() >= 2)
        {
            SdrPage* pPage = rPageView.GetPage();
            SdrObject* pObjA = pPage->GetObj(0);

            if(dynamic_cast<const SdrPathObj*>( pObjA))
            {
                basegfx::B2DPolyPolygon aPolyA(pObjA->GetPathPoly());
                aPolyA = basegfx::utils::correctOrientations(aPolyA);

                basegfx::B2DPolyPolygon aPolyB;

                for(sal_uInt32 a(1); a < rPageView.GetPage()->GetObjCount(); a++)
                {
                    SdrObject* pObjB = pPage->GetObj(a);

                    if(dynamic_cast<const SdrPathObj*>( pObjB))
                    {
                        basegfx::B2DPolyPolygon aCandidate(pObjB->GetPathPoly());
                        aCandidate = basegfx::utils::correctOrientations(aCandidate);
                        aPolyB.append(aCandidate);
                    }
                }

                if(aPolyA.count() && aPolyA.isClosed() && aPolyB.count())
                {
                    // poly A is the clipregion, clip poly b against it. Algo depends on
                    // poly b being closed.
                    basegfx::B2DPolyPolygon aResult(basegfx::utils::clipPolyPolygonOnPolyPolygon(aPolyB, aPolyA));

                    for(auto const& rPolygon : aResult)
                    {
                        int nR = comphelper::rng::uniform_int_distribution(0, 254);
                        int nG = comphelper::rng::uniform_int_distribution(0, 254);
                        int nB = comphelper::rng::uniform_int_distribution(0, 254);
                        Color aColor(nR, nG, nB);
                        impPaintStrokePolygon(rPolygon, rOutDev, aColor);
                    }
                }
            }
        }
    }
} // end of anonymous namespace
#endif // CLIPPER_TEST


void SdrPageWindow::RedrawAll( sdr::contact::ViewObjectContactRedirector* pRedirector )
{
    // set Redirector
    GetObjectContact().SetViewObjectContactRedirector(pRedirector);

    // set PaintingPageView
    const SdrView& rView = mpImpl->mrPageView.GetView();
    SdrModel& rModel = *(rView.GetModel());

    // get to be processed layers
    const bool bPrinter(GetPaintWindow().OutputToPrinter());
    SdrLayerIDSet aProcessLayers = bPrinter ? mpImpl->mrPageView.GetPrintableLayers() : mpImpl->mrPageView.GetVisibleLayers();

    // create PaintInfoRec; use Rectangle only temporarily
    const vcl::Region& rRegion = GetPaintWindow().GetRedrawRegion();

    // create processing data
    sdr::contact::DisplayInfo aDisplayInfo;

    // Draw all layers. do NOT draw form layer from CompleteRedraw, this is done separately
    // as a single layer paint
    const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
    const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName());
    aProcessLayers.Clear(nControlLayerId);

    // still something to paint?
    if(!aProcessLayers.IsEmpty())
    {
        aDisplayInfo.SetProcessLayers(aProcessLayers);

        // Set region as redraw area
        aDisplayInfo.SetRedrawArea(rRegion);

        // Draw/Impress
        aDisplayInfo.SetPageProcessingActive(rView.IsPagePaintingAllowed()); // #i72889#

        // paint page
        GetObjectContact().ProcessDisplay(aDisplayInfo);
    }

    // reset redirector
    GetObjectContact().SetViewObjectContactRedirector(nullptr);

    // LineClip test
#ifdef CLIPPER_TEST
    if(true)
    {
        impTryTest(GetPageView(), GetPaintWindow().GetOutputDevice());
    }
#endif // CLIPPER_TEST
}

void SdrPageWindow::RedrawLayer(const SdrLayerID* pId,
        sdr::contact::ViewObjectContactRedirector* pRedirector,
        basegfx::B2IRectangle const*const pPageFrame)
{
    // set redirector
    GetObjectContact().SetViewObjectContactRedirector(pRedirector);

    // set PaintingPageView
    const SdrView& rView = mpImpl->mrPageView.GetView();
    SdrModel& rModel = *(rView.GetModel());

    // get the layers to process
    const bool bPrinter(GetPaintWindow().OutputToPrinter());
    SdrLayerIDSet aProcessLayers = bPrinter ? mpImpl->mrPageView.GetPrintableLayers() : mpImpl->mrPageView.GetVisibleLayers();

    // is the given layer visible at all?
    if(aProcessLayers.IsSet(*pId))
    {
        // find out if we are painting the ControlLayer
        const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
        const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName());
        const bool bControlLayerProcessingActive(nControlLayerId == *pId);

        // create PaintInfoRec, use Rectangle only temporarily
        const vcl::Region& rRegion = GetPaintWindow().GetRedrawRegion();

        // create processing data
        sdr::contact::DisplayInfo aDisplayInfo;

        // is it the control layer? If Yes, set flag
        aDisplayInfo.SetControlLayerProcessingActive(bControlLayerProcessingActive);

        // Draw just the one given layer
        aProcessLayers.ClearAll();
        aProcessLayers.Set(*pId);

        aDisplayInfo.SetProcessLayers(aProcessLayers);

        // Set region as redraw area
        aDisplayInfo.SetRedrawArea(rRegion);

        // Writer or calc, coming from original RedrawOneLayer.
        // #i72889# no page painting for layer painting
        aDisplayInfo.SetPageProcessingActive(false);

        if (pPageFrame) // Writer page frame for anchor based clipping
        {
            aDisplayInfo.SetWriterPageFrame(*pPageFrame);
        }

        // paint page
        GetObjectContact().ProcessDisplay(aDisplayInfo);
    }

    // reset redirector
    GetObjectContact().SetViewObjectContactRedirector(nullptr);
}

// Invalidate call, used from ObjectContact(OfPageView) in InvalidatePartOfView(...)
void SdrPageWindow::InvalidatePageWindow(const basegfx::B2DRange& rRange)
{
    if (GetPageView().IsVisible() && GetPaintWindow().OutputToWindow())
    {
        OutputDevice& rWindow(GetPaintWindow().GetOutputDevice());
        basegfx::B2DRange aDiscreteRange(rRange);
        aDiscreteRange.transform(rWindow.GetViewTransformation());

        if (SvtOptionsDrawinglayer::IsAntiAliasing())
        {
            // invalidate one discrete unit more under the assumption that AA
            // needs one pixel more
            aDiscreteRange.grow(1.0);
        }

        // If the shapes use negative X coordinates, make them positive before sending
        // the invalidation rectangle.
        bool bNegativeX = mpImpl->mrPageView.GetView().IsNegativeX();

        const tools::Rectangle aVCLDiscreteRectangle(
            static_cast<tools::Long>(bNegativeX ? std::max(0.0, ceil(-aDiscreteRange.getMaxX())) : floor(aDiscreteRange.getMinX())),
            static_cast<tools::Long>(floor(aDiscreteRange.getMinY())),
            static_cast<tools::Long>(bNegativeX ? std::max(0.0, floor(-aDiscreteRange.getMinX())) : ceil(aDiscreteRange.getMaxX())),
            static_cast<tools::Long>(ceil(aDiscreteRange.getMaxY())));

        const bool bWasMapModeEnabled(rWindow.IsMapModeEnabled());
        rWindow.EnableMapMode(false);
        GetPageView().GetView().InvalidateOneWin(rWindow, aVCLDiscreteRectangle);
        rWindow.EnableMapMode(bWasMapModeEnabled);
    }
    else if (comphelper::LibreOfficeKit::isActive())
    {
        // we don't really have to have a paint window with LOK; OTOH we know
        // that the drawinglayer units are 100ths of mm, so they are easy to
        // convert to twips

        // If the shapes use negative X coordinates, make them positive before sending
        // the invalidation rectangle.
        bool bNegativeX = mpImpl->mrPageView.GetView().IsNegativeX();
        const tools::Rectangle aRect100thMM(
            static_cast<tools::Long>(bNegativeX ? std::max(0.0, ceil(-rRange.getMaxX())) : floor(rRange.getMinX())),
            static_cast<tools::Long>(floor(rRange.getMinY())),
            static_cast<tools::Long>(bNegativeX ? std::max(0.0, floor(-rRange.getMinX())) : ceil(rRange.getMaxX())),
            static_cast<tools::Long>(ceil(rRange.getMaxY())));

        const tools::Rectangle aRectTwips = o3tl::convert(aRect100thMM, o3tl::Length::mm100, o3tl::Length::twip);

        if (SfxViewShell* pViewShell = SfxViewShell::Current())
            SfxLokHelper::notifyInvalidation(pViewShell, &aRectTwips);
    }
}

// ObjectContact section
const sdr::contact::ObjectContact& SdrPageWindow::GetObjectContact() const
{
    if (!mpImpl->mpObjectContact)
    {
        mpImpl->mpObjectContact = GetPageView().GetView().createViewSpecificObjectContact(
            const_cast<SdrPageWindow&>(*this),
            "svx::svdraw::SdrPageWindow mpObjectContact");
    }

    return *mpImpl->mpObjectContact;
}

sdr::contact::ObjectContact& SdrPageWindow::GetObjectContact()
{
    if (!mpImpl->mpObjectContact)
    {
        mpImpl->mpObjectContact = GetPageView().GetView().createViewSpecificObjectContact(
             *this,
             "svx::svdraw::SdrPageWindow mpObjectContact" );
    }

    return *mpImpl->mpObjectContact;
}

bool SdrPageWindow::HasObjectContact() const
{
    return mpImpl->mpObjectContact != nullptr;
}

// #i26631#
void SdrPageWindow::ResetObjectContact()
{
    if (mpImpl->mpObjectContact)
    {
        delete mpImpl->mpObjectContact;
        mpImpl->mpObjectContact = nullptr;
    }
}

void SdrPageWindow::SetDesignMode( bool _bDesignMode ) const
{
    const sdr::contact::ObjectContactOfPageView* pOC = dynamic_cast< const sdr::contact::ObjectContactOfPageView* >( &GetObjectContact() );
    DBG_ASSERT( pOC, "SdrPageWindow::SetDesignMode: invalid object contact!" );
    if ( pOC )
        pOC->SetUNOControlsDesignMode( _bDesignMode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
