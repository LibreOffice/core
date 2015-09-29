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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <osl/mutex.hxx>
#include <svx/fmview.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>



using namespace ::com::sun::star;

struct SdrPageWindow::Impl
{
    // #110094# ObjectContact section
    mutable sdr::contact::ObjectContactOfPageView* mpObjectContact;

    // the SdrPageView this window belongs to
    SdrPageView& mrPageView;

    // the PaintWindow to paint on. Here is access to OutDev etc.
    // #i72752# change to pointer to allow patcing it in DrawLayer() if necessary
    SdrPaintWindow* mpPaintWindow;
    SdrPaintWindow* mpOriginalPaintWindow;

    // UNO stuff for xControls
    uno::Reference<awt::XControlContainer> mxControlContainer;

    Impl( SdrPageView& rPageView, SdrPaintWindow& rPaintWindow ) :
        mpObjectContact(NULL),
        mrPageView(rPageView),
        mpPaintWindow(&rPaintWindow),
        mpOriginalPaintWindow(NULL)
    {
    }

    ~Impl()
    {
    }
};


uno::Reference<awt::XControlContainer> SdrPageWindow::GetControlContainer( bool _bCreateIfNecessary ) const
{
    if (!mpImpl->mxControlContainer.is() && _bCreateIfNecessary)
    {
        SdrView& rView = GetPageView().GetView();

        const SdrPaintWindow& rPaintWindow( GetOriginalPaintWindow() ? *GetOriginalPaintWindow() : GetPaintWindow() );
        if ( rPaintWindow.OutputToWindow() && !rView.IsPrintPreview() )
        {
            vcl::Window& rWindow = dynamic_cast< vcl::Window& >( rPaintWindow.GetOutputDevice() );
            const_cast< SdrPageWindow* >( this )->mpImpl->mxControlContainer = VCLUnoHelper::CreateControlContainer( &rWindow );

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
            const_cast< SdrPageWindow* >( this )->mpImpl->mxControlContainer = uno::Reference< awt::XControlContainer >(xFactory->createInstance("com.sun.star.awt.UnoControlContainer"), uno::UNO_QUERY);
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

    if (mpImpl->mxControlContainer.is())
    {
        SdrView& rView = GetPageView().GetView();

        // notify derived views
        FmFormView* pViewAsFormView = dynamic_cast< FmFormView* >( &rView );
        if ( pViewAsFormView )
            pViewAsFormView->RemoveControlContainer(mpImpl->mxControlContainer);

        // dispose the control container
        uno::Reference< lang::XComponent > xComponent(mpImpl->mxControlContainer, uno::UNO_QUERY);
        xComponent->dispose();
    }

    delete mpImpl;
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
rtl::Reference< sdr::overlay::OverlayManager > SdrPageWindow::GetOverlayManager() const
{
    return GetPaintWindow().GetOverlayManager();
}

void SdrPageWindow::patchPaintWindow(SdrPaintWindow& rPaintWindow)
{
    mpImpl->mpOriginalPaintWindow = mpImpl->mpPaintWindow;
    mpImpl->mpPaintWindow = &rPaintWindow;
}

void SdrPageWindow::unpatchPaintWindow()
{
    DBG_ASSERT(mpImpl->mpOriginalPaintWindow, "SdrPageWindow::unpatchPaintWindow: paint window not patched!" );
    if (mpImpl->mpOriginalPaintWindow)
    {
        mpImpl->mpPaintWindow = mpImpl->mpOriginalPaintWindow;
        mpImpl->mpOriginalPaintWindow = NULL;
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
            aCandidate = basegfx::tools::adaptiveSubdivideByAngle(rCandidate);
        }

        if(aCandidate.count())
        {
            const sal_uInt32 nLoopCount(aCandidate.isClosed() ? aCandidate.count() : aCandidate.count() - 1L);
            rOutDev.SetFillColor();
            rOutDev.SetLineColor(aColor);

            for(sal_uInt32 a(0L); a < nLoopCount; a++)
            {
                const basegfx::B2DPoint aBStart(aCandidate.getB2DPoint(a));
                const basegfx::B2DPoint aBEnd(aCandidate.getB2DPoint((a + 1) % aCandidate.count()));
                const Point aStart(FRound(aBStart.getX()), FRound(aBStart.getY()));
                const Point aEnd(FRound(aBEnd.getX()), FRound(aBEnd.getY()));
                rOutDev.DrawLine(aStart, aEnd);
            }
        }
    }

    void impTryTest(const SdrPageView& rPageView, OutputDevice& rOutDev)
    {
        if(rPageView.GetPage() && rPageView.GetPage()->GetObjCount() >= 2L)
        {
            SdrPage* pPage = rPageView.GetPage();
            SdrObject* pObjA = pPage->GetObj(0L);

            if(pObjA && dynamic_cast<const SdrPathObj*>( pObjA) !=  nullptr)
            {
                basegfx::B2DPolyPolygon aPolyA(pObjA->GetPathPoly());
                aPolyA = basegfx::tools::correctOrientations(aPolyA);

                basegfx::B2DPolyPolygon aPolyB;

                for(sal_uInt32 a(1L); a < rPageView.GetPage()->GetObjCount(); a++)
                {
                    SdrObject* pObjB = pPage->GetObj(a);

                    if(pObjB && dynamic_cast<const SdrPathObj*>( pObjB) !=  nullptr)
                    {
                        basegfx::B2DPolyPolygon aCandidate(pObjB->GetPathPoly());
                        aCandidate = basegfx::tools::correctOrientations(aCandidate);
                        aPolyB.append(aCandidate);
                    }
                }

                if(aPolyA.count() && aPolyA.isClosed() && aPolyB.count())
                {
                    // poly A is the clipregion, clip poly b against it. Algo depends on
                    // poly b being closed.
                    basegfx::B2DPolyPolygon aResult(basegfx::tools::clipPolyPolygonOnPolyPolygon(aPolyB, aPolyA));

                    for(sal_uInt32 a(0L); a < aResult.count(); a++)
                    {
                        int nR = comphelper::rng::uniform_int_distribution(0, 254);
                        int nG = comphelper::rng::uniform_int_distribution(0, 254);
                        int nB = comphelper::rng::uniform_int_distribution(0, 254);
                        Color aColor(nR, nG, nB);
                        impPaintStrokePolygon(aResult.getB2DPolygon(a), rOutDev, aColor);
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
    SetOfByte aProcessLayers = bPrinter ? mpImpl->mrPageView.GetPrintableLayers() : mpImpl->mrPageView.GetVisibleLayers();

    // create PaintInfoRec; use Rectangle only temporarily
    const vcl::Region& rRegion = GetPaintWindow().GetRedrawRegion();

    // create processing data
    sdr::contact::DisplayInfo aDisplayInfo;

    // Draw all layers. do NOT draw form layer from CompleteRedraw, this is done separately
    // as a single layer paint
    const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
    const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), false);
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
    GetObjectContact().SetViewObjectContactRedirector(0L);

    // LineClip test
#ifdef CLIPPER_TEST
    if(true)
    {
        impTryTest(GetPageView(), GetPaintWindow().GetOutputDevice());
    }
#endif // CLIPPER_TEST
}

void SdrPageWindow::RedrawLayer( const SdrLayerID* pId, sdr::contact::ViewObjectContactRedirector* pRedirector )
{
    // set redirector
    GetObjectContact().SetViewObjectContactRedirector(pRedirector);

    // set PaintingPageView
    const SdrView& rView = mpImpl->mrPageView.GetView();
    SdrModel& rModel = *(rView.GetModel());

    // get the layers to process
    const bool bPrinter(GetPaintWindow().OutputToPrinter());
    SetOfByte aProcessLayers = bPrinter ? mpImpl->mrPageView.GetPrintableLayers() : mpImpl->mrPageView.GetVisibleLayers();

    // is the given layer visible at all?
    if(aProcessLayers.IsSet(*pId))
    {
        // find out if we are painting the ControlLayer
        const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
        const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), false);
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

        // paint page
        GetObjectContact().ProcessDisplay(aDisplayInfo);
    }

    // reset redirector
    GetObjectContact().SetViewObjectContactRedirector(0L);
}

// Invalidate call, used from ObjectContact(OfPageView) in InvalidatePartOfView(...)
void SdrPageWindow::InvalidatePageWindow(const basegfx::B2DRange& rRange)
{
    if(GetPageView().IsVisible() && GetPaintWindow().OutputToWindow())
    {
        const SvtOptionsDrawinglayer aDrawinglayerOpt;
        vcl::Window& rWindow(static_cast< vcl::Window& >(GetPaintWindow().GetOutputDevice()));
        basegfx::B2DRange aDiscreteRange(rRange);
        aDiscreteRange.transform(rWindow.GetViewTransformation());

        if(aDrawinglayerOpt.IsAntiAliasing())
        {
            // invalidate one discrete unit more under the assumption that AA
            // needs one pixel more
            aDiscreteRange.grow(1.0);
        }

        const Rectangle aVCLDiscreteRectangle(
            static_cast<long>(floor(aDiscreteRange.getMinX())),
            static_cast<long>(floor(aDiscreteRange.getMinY())),
            static_cast<long>(ceil(aDiscreteRange.getMaxX())),
            static_cast<long>(ceil(aDiscreteRange.getMaxY())));
        const bool bWasMapModeEnabled(rWindow.IsMapModeEnabled());

        rWindow.EnableMapMode(false);
        rWindow.Invalidate(aVCLDiscreteRectangle, InvalidateFlags::NoErase);
        rWindow.EnableMapMode(bWasMapModeEnabled);
    }
}

// ObjectContact section
const sdr::contact::ObjectContact& SdrPageWindow::GetObjectContact() const
{
    if (!mpImpl->mpObjectContact)
        mpImpl->mpObjectContact = new sdr::contact::ObjectContactOfPageView(const_cast<SdrPageWindow&>(*this));

    return *mpImpl->mpObjectContact;
}

sdr::contact::ObjectContact& SdrPageWindow::GetObjectContact()
{
    if (!mpImpl->mpObjectContact)
        mpImpl->mpObjectContact = new sdr::contact::ObjectContactOfPageView(*this);

    return *mpImpl->mpObjectContact;
}

bool SdrPageWindow::HasObjectContact() const
{
    return mpImpl->mpObjectContact != NULL;
}

// #i26631#
void SdrPageWindow::ResetObjectContact()
{
    if (mpImpl->mpObjectContact)
    {
        delete mpImpl->mpObjectContact;
        mpImpl->mpObjectContact = 0L;
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
