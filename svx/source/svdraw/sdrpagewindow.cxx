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

#include <svx/sdrpagewindow.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <osl/mutex.hxx>
#include <svx/fmview.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > SdrPageWindow::GetControlContainer( bool _bCreateIfNecessary ) const
{
    if ( !mxControlContainer.is() && _bCreateIfNecessary )
    {
        SdrView& rView = GetPageView().GetView();

        const SdrPaintWindow& rPaintWindow( GetOriginalPaintWindow() ? *GetOriginalPaintWindow() : GetPaintWindow() );
        if ( rPaintWindow.OutputToWindow() && !rView.IsPrintPreview() )
        {
            Window& rWindow = dynamic_cast< Window& >( rPaintWindow.GetOutputDevice() );
            const_cast< SdrPageWindow* >( this )->mxControlContainer = VCLUnoHelper::CreateControlContainer( &rWindow );

            // #100394# xC->setVisible triggers window->Show() and this has
            // problems when the view is not completely constructed which may
            // happen when loading. This leads to accessibility broadcasts which
            // throw asserts due to the not finished view. All this chain can be avoided
            // since xC->setVisible is here called only for the side effect in
            // UnoControlContainer::setVisible(...) which calls createPeer(...).
            // This will now be called directly from here.

            uno::Reference< awt::XControl > xControl(mxControlContainer, uno::UNO_QUERY);
            if(xControl.is())
            {
                uno::Reference< uno::XInterface > xContext = xControl->getContext();
                if(!xContext.is())
                {
                    xControl->createPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > (),
                        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > () );
                }
            }
        }
        else
        {
            // Printer and VirtualDevice, or rather: no OutDev
            uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            if( xFactory.is() )
            {
                const_cast< SdrPageWindow* >( this )->mxControlContainer = uno::Reference< awt::XControlContainer >(xFactory->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlContainer"))), uno::UNO_QUERY);
                uno::Reference< awt::XControlModel > xModel(xFactory->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlContainerModel"))), uno::UNO_QUERY);
                uno::Reference< awt::XControl > xControl(mxControlContainer, uno::UNO_QUERY);
                if (xControl.is())
                    xControl->setModel(xModel);

                OutputDevice& rOutDev = rPaintWindow.GetOutputDevice();
                Point aPosPix = rOutDev.GetMapMode().GetOrigin();
                Size aSizePix = rOutDev.GetOutputSizePixel();

                uno::Reference< awt::XWindow > xContComp(mxControlContainer, uno::UNO_QUERY);
                if( xContComp.is() )
                    xContComp->setPosSize(aPosPix.X(), aPosPix.Y(), aSizePix.Width(), aSizePix.Height(), awt::PosSize::POSSIZE);
            }
        }

        FmFormView* pViewAsFormView = dynamic_cast< FmFormView* >( &rView );
        if ( pViewAsFormView )
            pViewAsFormView->InsertControlContainer(mxControlContainer);
    }
    return mxControlContainer;
}

SdrPageWindow::SdrPageWindow(SdrPageView& rPageView, SdrPaintWindow& rPaintWindow)
:   mpObjectContact(0L),
    mrPageView(rPageView),
    mpPaintWindow(&rPaintWindow),
    mpOriginalPaintWindow(NULL)
{
}

SdrPageWindow::~SdrPageWindow()
{
    // #i26631#
    ResetObjectContact();

    if (mxControlContainer.is())
    {
        SdrView& rView = GetPageView().GetView();

        // notify derived views
        FmFormView* pViewAsFormView = dynamic_cast< FmFormView* >( &rView );
        if ( pViewAsFormView )
            pViewAsFormView->RemoveControlContainer(mxControlContainer);

        // dispose the control container
        uno::Reference< lang::XComponent > xComponent(mxControlContainer, uno::UNO_QUERY);
        xComponent->dispose();
    }
}

// ObjectContact section
sdr::contact::ObjectContact* SdrPageWindow::CreateViewSpecificObjectContact()
{
    return new sdr::contact::ObjectContactOfPageView(*this);
}

// OVERLAY MANAGER
rtl::Reference< ::sdr::overlay::OverlayManager > SdrPageWindow::GetOverlayManager() const
{
    return GetPaintWindow().GetOverlayManager();
}

void SdrPageWindow::patchPaintWindow(SdrPaintWindow& rPaintWindow)
{
    mpOriginalPaintWindow = mpPaintWindow;
    mpPaintWindow = &rPaintWindow;
}

void SdrPageWindow::unpatchPaintWindow()
{
    DBG_ASSERT(mpOriginalPaintWindow, "SdrPageWindow::unpatchPaintWindow: paint window not patched!" );
    if ( mpOriginalPaintWindow )
    {
        mpPaintWindow = mpOriginalPaintWindow;
        mpOriginalPaintWindow = NULL;
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

void SdrPageWindow::PrepareRedraw(const Region& rReg)
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

//////////////////////////////////////////////////////////////////////////////
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

            if(pObjA && pObjA->ISA(SdrPathObj))
            {
                basegfx::B2DPolyPolygon aPolyA(((SdrPathObj*)pObjA)->GetPathPoly());
                aPolyA = basegfx::tools::correctOrientations(aPolyA);

                basegfx::B2DPolyPolygon aPolyB;

                for(sal_uInt32 a(1L); a < rPageView.GetPage()->GetObjCount(); a++)
                {
                    SdrObject* pObjB = pPage->GetObj(a);

                    if(pObjB && pObjB->ISA(SdrPathObj))
                    {
                        basegfx::B2DPolyPolygon aCandidate(((SdrPathObj*)pObjB)->GetPathPoly());
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
                        Color aColor(rand()%255, rand()%255, rand()%255);
                        impPaintStrokePolygon(aResult.getB2DPolygon(a), rOutDev, aColor);
                    }
                }
            }
        }
    }
} // end of anonymous namespace
#endif // CLIPPER_TEST

//////////////////////////////////////////////////////////////////////////////

void SdrPageWindow::RedrawAll(sdr::contact::ViewObjectContactRedirector* pRedirector) const
{
    // set Redirector
    GetObjectContact().SetViewObjectContactRedirector(pRedirector);

    // set PaintingPageView
    const SdrView& rView = mrPageView.GetView();
    SdrModel& rModel = *((SdrModel*)rView.GetModel());

    // get to be processed layers
    const sal_Bool bPrinter(GetPaintWindow().OutputToPrinter());
    SetOfByte aProcessLayers = bPrinter ? mrPageView.GetPrintableLayers() : mrPageView.GetVisibleLayers();

    // create PaintInfoRec; use Rectangle only temporarily
    const Region& rRegion = GetPaintWindow().GetRedrawRegion();

    // create processing data
    sdr::contact::DisplayInfo aDisplayInfo;

    // Draw all layers. do NOT draw form layer from CompleteRedraw, this is done separately
    // as a single layer paint
    const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
    const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), sal_False);
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

void SdrPageWindow::RedrawLayer(const SdrLayerID* pId, sdr::contact::ViewObjectContactRedirector* pRedirector) const
{
    // set redirector
    GetObjectContact().SetViewObjectContactRedirector(pRedirector);

    // set PaintingPageView
    const SdrView& rView = mrPageView.GetView();
    SdrModel& rModel = *((SdrModel*)rView.GetModel());

    // get the layers to process
    const sal_Bool bPrinter(GetPaintWindow().OutputToPrinter());
    SetOfByte aProcessLayers = bPrinter ? mrPageView.GetPrintableLayers() : mrPageView.GetVisibleLayers();

    // is the given layer visible at all?
    if(aProcessLayers.IsSet(*pId))
    {
        // find out if we are painting the ControlLayer
        const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
        const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), sal_False);
        const sal_Bool bControlLayerProcessingActive(pId && nControlLayerId == *pId);

        // create PaintInfoRec, use Rectangle only temporarily
        const Region& rRegion = GetPaintWindow().GetRedrawRegion();

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
        Window& rWindow(static_cast< Window& >(GetPaintWindow().GetOutputDevice()));
        basegfx::B2DRange aDiscreteRange(rRange);
        aDiscreteRange.transform(rWindow.GetViewTransformation());

        if(aDrawinglayerOpt.IsAntiAliasing())
        {
            // invalidate one discrete unit more under the assumption that AA
            // needs one pixel more
            aDiscreteRange.grow(1.0);
        }

        const Rectangle aVCLDiscreteRectangle(
                (sal_Int32)floor(aDiscreteRange.getMinX()), (sal_Int32)floor(aDiscreteRange.getMinY()),
                (sal_Int32)ceil(aDiscreteRange.getMaxX()), (sal_Int32)ceil(aDiscreteRange.getMaxY()));
        const bool bWasMapModeEnabled(rWindow.IsMapModeEnabled());

        rWindow.EnableMapMode(false);
        rWindow.Invalidate(aVCLDiscreteRectangle, INVALIDATE_NOERASE);
        rWindow.EnableMapMode(bWasMapModeEnabled);
    }
}

// ObjectContact section
sdr::contact::ObjectContact& SdrPageWindow::GetObjectContact() const
{
    if(!mpObjectContact)
    {
        ((SdrPageWindow*)this)->mpObjectContact = ((SdrPageWindow*)this)->CreateViewSpecificObjectContact();
    }

    return *mpObjectContact;
}

bool SdrPageWindow::HasObjectContact() const
{
    return ( mpObjectContact != NULL );
}

// #i26631#
void SdrPageWindow::ResetObjectContact()
{
    if(mpObjectContact)
    {
        delete mpObjectContact;
        mpObjectContact = 0L;
    }
}

void SdrPageWindow::SetDesignMode( bool _bDesignMode ) const
{
    const ::sdr::contact::ObjectContactOfPageView* pOC = dynamic_cast< const ::sdr::contact::ObjectContactOfPageView* >( &GetObjectContact() );
    DBG_ASSERT( pOC, "SdrPageWindow::SetDesignMode: invalid object contact!" );
    if ( pOC )
        pOC->SetUNOControlsDesignMode( _bDesignMode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
