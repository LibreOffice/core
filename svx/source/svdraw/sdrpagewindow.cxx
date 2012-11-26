/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
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
#include <vos/mutex.hxx>
#include <svx/fmview.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// clip test

#ifdef CLIPPER_TEST
#include <svx/svdopath.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <vcl/salbtype.hxx>     // FRound
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <algorithm>
#endif // CLIPPER_TEST

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
            // throw asserts due to the not finished view. All this chan be avoided
            // since xC->setVisible is here called only for the side effect in
            // UnoControlContainer::setVisible(...) which calls createPeer(...).
            // This will now be called directly from here.

            // UnoContainerModel erzeugen
            // uno::Reference< awt::XWindow > xC(mxControlContainer, uno::UNO_QUERY);
            // CreateControlContainer() is only used from
            // , thus it seems not necessary to make
            // it visible her at all.
            // #58917# Das Show darf nicht am VCL-Fenster landen, weil dann Assertion vom SFX
            // sal_Bool bVis = pWindow->IsVisible();
            // xC->setVisible(sal_True);
            // if ( !bVis )
            //  pWindow->Hide();
            //  if( !mxContext.is() && bVisible )
            //      // Es ist ein TopWindow, also automatisch anzeigen
            //      createPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > (), ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > () );

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
            // Printer und VirtualDevice, bzw. kein OutDev
            uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            if( xFactory.is() )
            {
                const_cast< SdrPageWindow* >( this )->mxControlContainer = uno::Reference< awt::XControlContainer >(xFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlContainer")), uno::UNO_QUERY);
                uno::Reference< awt::XControlModel > xModel(xFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlContainerModel")), uno::UNO_QUERY);
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
    // #110094#, #i26631#
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

// #110094# ObjectContact section
sdr::contact::ObjectContact* SdrPageWindow::CreateViewSpecificObjectContact()
{
    return new sdr::contact::ObjectContactOfPageView(*this);
}

// OVERLAYMANAGER
::sdr::overlay::OverlayManager* SdrPageWindow::GetOverlayManager() const
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

void SdrPageWindow::PostPaint()
{
}

void SdrPageWindow::PrepareRedraw(const Region& rReg)
{
    // evtl. give OC the chance to do ProcessDisplay preparations
    if(HasObjectContact())
    {
        GetObjectContact().PrepareProcessDisplay();
    }

    // remember eventually changed RedrawArea at PaintWindow for usage with
    // overlay and PreRenderDevice stuff
    GetPaintWindow().SetRedrawRegion(rReg);
}

//////////////////////////////////////////////////////////////////////////////
// clip test

#ifdef CLIPPER_TEST
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
        if(rPageView.GetPage() && rPageView.GetPage()->GetObjCount() >= 2)
        {
            SdrPage* pPage = rPageView.GetPage();
            SdrPathObj* pObjA = dynamic_cast< SdrPathObj* >(pPage->GetObj(0));

            if(pObjA)
            {
                basegfx::B2DPolyPolygon aPolyA(pObjA->getB2DPolyPolygonInObjectCoordinates());
                aPolyA = basegfx::tools::correctOrientations(aPolyA);

                basegfx::B2DPolyPolygon aPolyB;

                for(sal_uInt32 a(1L); a < rPageView.GetPage()->GetObjCount(); a++)
                {
                    SdrPathObj* pObjB = dynamic_cast< SdrPathObj* >(pPage->GetObj(a));

                    if(pObjB)
                    {
                        basegfx::B2DPolyPolygon aCandidate(pObjB->getB2DPolyPolygonInObjectCoordinates());
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

                    bool bBla = true;
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

    // get to be processed layers
    const sal_Bool bPrinter(GetPaintWindow().OutputToPrinter());
    SetOfByte aProcessLayers = bPrinter ? mrPageView.GetPrintableLayers() : mrPageView.GetVisibleLayers();

    // create PaintInfoRec, #114359# use Rectangle only temporarily
    const Region& rRegion = GetPaintWindow().GetRedrawRegion();

    // create processing data
    sdr::contact::DisplayInfo aDisplayInfo;

    // Draw all layers. do NOT draw form layer from CompleteRedraw, this is done separate
    // as a single layer paint
    const SdrLayerAdmin& rLayerAdmin = rView.getSdrModelFromSdrView().GetModelLayerAdmin();
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

    // get the layers to process
    const sal_Bool bPrinter(GetPaintWindow().OutputToPrinter());
    SetOfByte aProcessLayers = bPrinter ? mrPageView.GetPrintableLayers() : mrPageView.GetVisibleLayers();

    // is the given layer visible at all?
    if(aProcessLayers.IsSet(*pId))
    {
        // find out if we are painting the ControlLayer
        const SdrLayerAdmin& rLayerAdmin = rView.getSdrModelFromSdrView().GetModelLayerAdmin();
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
        basegfx::B2DRange aDiscreteRange(rWindow.GetViewTransformation() * rRange);

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

// #110094# ObjectContact section
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

void SdrPageWindow::SetDesignMode( bool _bDesignMode )
{
    ::sdr::contact::ObjectContactOfPageView* pOC = dynamic_cast< ::sdr::contact::ObjectContactOfPageView* >( &GetObjectContact() );
    DBG_ASSERT( pOC, "SdrPageWindow::SetDesignMode: invalid object contact!" );
    if ( pOC )
        pOC->SetUNOControlsDesignMode( _bDesignMode );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
