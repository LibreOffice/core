/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrpagewindow.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 14:53:00 $
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

#ifndef _SDRPAGEWINDOW_HXX
#include <svx/sdrpagewindow.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODECHANGEBROADCASTER_HPP_
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _SVDOUNO_HXX
#include <svx/svdouno.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#ifndef _SDRPAINTWINDOW_HXX
#include <sdrpaintwindow.hxx>
#endif

#ifndef _SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#endif

#ifndef _XOUTX_HXX
#include <svx/xoutx.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SVX_FMVIEW_HXX
#include <svx/fmview.hxx>
#endif

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
            // BOOL bVis = pWindow->IsVisible();
            // xC->setVisible(TRUE);
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

    if(mpObjectContact)
    {
        mpObjectContact->PrepareDelete();
        delete mpObjectContact;
        mpObjectContact = 0L;
    }

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

SdrPaintInfoRec* SdrPageWindow::ImpCreateNewPageInfoRec(const Rectangle& rDirtyRect,
    sal_uInt16 nPaintMode, const SdrLayerID* pId) const
{
    SdrPaintInfoRec* pInfoRec = new SdrPaintInfoRec();
    const sal_Bool bPrinter(GetPaintWindow().OutputToPrinter());
    Rectangle aCheckRect(rDirtyRect);
    Size a1PixSiz(GetPaintWindow().GetOutputDevice().PixelToLogic(Size(1, 1)));

    aCheckRect.Left() -= a1PixSiz.Width();
    aCheckRect.Top() -= a1PixSiz.Height();
    aCheckRect.Right() += a1PixSiz.Width();
    aCheckRect.Bottom() += a1PixSiz.Height();

    pInfoRec->pPV = &mrPageView;
    pInfoRec->bPrinter = bPrinter;
    pInfoRec->aDirtyRect = rDirtyRect;
    pInfoRec->aCheckRect = aCheckRect;

    if(pId)
    {
        pInfoRec->aPaintLayer.ClearAll();
        pInfoRec->aPaintLayer.Set(*pId);
    }
    else
    {
        pInfoRec->aPaintLayer = bPrinter
            ? mrPageView.GetPrintableLayers()
            : mrPageView.GetVisibleLayers();
    }

    pInfoRec->nPaintMode = nPaintMode;

    if(mrPageView.GetObjList() != mrPageView.GetPage())
    {
        pInfoRec->pAktList = mrPageView.GetObjList();
    }

    return pInfoRec;
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

void SdrPageWindow::PrepareRedraw(const Region& rReg)
{
    // get XOutDev
    const SdrView& rView = mrPageView.GetView();
    XOutputDevice* pXOut = rView.GetExtendedOutputDevice();

    // get to be processed layers
    const sal_Bool bPrinter(GetPaintWindow().OutputToPrinter());
    SetOfByte aProcessLayers = bPrinter ? mrPageView.GetPrintableLayers() : mrPageView.GetVisibleLayers();

    // get OutDev and set offset there
    OutputDevice& rTargetDevice = GetPaintWindow().GetOutputDevice();
    pXOut->SetOutDev(&rTargetDevice);
    pXOut->SetOffset(Point());

    // create PaintInfoRec, use Rectangle only temporarily
    SdrPaintInfoRec* pInfoRec = ImpCreateNewPageInfoRec(rReg.GetBoundRect(), 0, 0L);

    // create processing data
    sdr::contact::DisplayInfo aDisplayInfo(&mrPageView);

    aDisplayInfo.SetProcessLayers(aProcessLayers);
    aDisplayInfo.SetExtendedOutputDevice(pXOut);
    aDisplayInfo.SetPaintInfoRec(pInfoRec);
    aDisplayInfo.SetOutputDevice(&rTargetDevice);

    // #114359# Set region as redraw area, not a rectangle
    aDisplayInfo.SetRedrawArea(rReg);

    // no PagePainting for preparations
    aDisplayInfo.SetPagePainting(rView.IsPagePaintingAllowed()); // #i72889#

    // remember eventually changed RedrawArea at PaintWindow for usage with
    // overlay and PreRenderDevice stuff
    GetPaintWindow().SetRedrawRegion(aDisplayInfo.GetRedrawArea());

    // delete PaintInfoRec
    delete pInfoRec;
}

//////////////////////////////////////////////////////////////////////////////
// clip test
#ifdef CLIPPER_TEST

#ifndef _SVDOPATH_HXX
#include <svx/svdopath.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif

#ifndef _BGFX_POLYGON_CUTANDTOUCH_HXX
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#endif

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

                    bool bBla = true;
                }
            }
        }
    }
} // end of anonymous namespace
#endif // CLIPPER_TEST

//////////////////////////////////////////////////////////////////////////////

void SdrPageWindow::RedrawAll(sal_uInt16 nPaintMode, ::sdr::contact::ViewObjectContactRedirector* pRedirector) const
{
    // set Redirector
    GetObjectContact().SetViewObjectContactRedirector(pRedirector);

    // set PaintingPageView
    const SdrView& rView = mrPageView.GetView();
    SdrModel& rModel = *((SdrModel*)rView.GetModel());
    rModel.SetPaintingPageView(&mrPageView);

    // get XOutDev
    XOutputDevice* pXOut = rView.GetExtendedOutputDevice();

    // get to be processed layers
    const sal_Bool bPrinter(GetPaintWindow().OutputToPrinter());
    SetOfByte aProcessLayers = bPrinter ? mrPageView.GetPrintableLayers() : mrPageView.GetVisibleLayers();

    // get TargetDevice and force output to this one given target
    SdrPreRenderDevice* pPreRenderDevice = GetPaintWindow().GetPreRenderDevice();
    OutputDevice& rTargetDevice = (pPreRenderDevice)
        ? pPreRenderDevice->GetPreRenderDevice() : GetPaintWindow().GetOutputDevice();
    pXOut->SetOutDev(&rTargetDevice);

    // set Offset at XOutDev
    pXOut->SetOffset(Point());

    // create PaintInfoRec, #114359# use Rectangle only temporarily
    const Region& rRegion = GetPaintWindow().GetRedrawRegion();
    SdrPaintInfoRec* pInfoRec = ImpCreateNewPageInfoRec(rRegion.GetBoundRect(), nPaintMode, 0L);

    // create processing data
    sdr::contact::DisplayInfo aDisplayInfo(&mrPageView);

    // Draw all layers. do NOT draw form layer from CompleteRedraw, this is done separate
    // as a single layer paint
    const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
    const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), sal_False);
    aProcessLayers.Clear(nControlLayerId);

    // still something to paint?
    if(!aProcessLayers.IsEmpty())
    {
        aDisplayInfo.SetProcessLayers(aProcessLayers);
        aDisplayInfo.SetExtendedOutputDevice(pXOut);
        aDisplayInfo.SetPaintInfoRec(pInfoRec);
        aDisplayInfo.SetOutputDevice(&rTargetDevice);

        // Set region as redraw area
        aDisplayInfo.SetRedrawArea(rRegion);

        // Draw/Impress
        aDisplayInfo.SetPagePainting(rView.IsPagePaintingAllowed()); // #i72889#

        // paint page
        GetObjectContact().ProcessDisplay(aDisplayInfo);
    }

    // delete PaintInfoRec
    delete pInfoRec;

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

void SdrPageWindow::RedrawLayer(
    sal_uInt16 nPaintMode, const SdrLayerID* pId,
    ::sdr::contact::ViewObjectContactRedirector* pRedirector) const
{
    // set redirector
    GetObjectContact().SetViewObjectContactRedirector(pRedirector);

    // set PaintingPageView
    const SdrView& rView = mrPageView.GetView();
    SdrModel& rModel = *((SdrModel*)rView.GetModel());
    rModel.SetPaintingPageView(&mrPageView);

    // get XOutDev
    XOutputDevice* pXOut = rView.GetExtendedOutputDevice();

    // get the layers to process
    const sal_Bool bPrinter(GetPaintWindow().OutputToPrinter());
    SetOfByte aProcessLayers = bPrinter ? mrPageView.GetPrintableLayers() : mrPageView.GetVisibleLayers();

    // is the given layer visible at all?
    if(aProcessLayers.IsSet(*pId))
    {
        // find out if we are painting the ControlLayer
        const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
        const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), sal_False);
        const sal_Bool bControlLayerPainting(pId && nControlLayerId == *pId);

        // Get PreRenderDevice and force output to this one given target
        SdrPreRenderDevice* pPreRenderDevice = GetPaintWindow().GetPreRenderDevice();
        OutputDevice& rTargetDevice =
            (pPreRenderDevice && !bControlLayerPainting)
            ? pPreRenderDevice->GetPreRenderDevice() : GetPaintWindow().GetOutputDevice();
        pXOut->SetOutDev(&rTargetDevice);

        // set offset at XOutDev
        pXOut->SetOffset(Point());

        // create PaintInfoRec, use Rectangle only temporarily
        const Region& rRegion = GetPaintWindow().GetRedrawRegion();
        SdrPaintInfoRec* pInfoRec = ImpCreateNewPageInfoRec(rRegion.GetBoundRect(), nPaintMode, pId);

        // create processing data
        sdr::contact::DisplayInfo aDisplayInfo(&mrPageView);

        // is it the control layer? If Yes, set flag
        aDisplayInfo.SetControlLayerPainting(bControlLayerPainting);

        // Draw just the one given layer
        aProcessLayers.ClearAll();
        aProcessLayers.Set(*pId);

        aDisplayInfo.SetProcessLayers(aProcessLayers);
        aDisplayInfo.SetExtendedOutputDevice(pXOut);
        aDisplayInfo.SetPaintInfoRec(pInfoRec);
        aDisplayInfo.SetOutputDevice(&rTargetDevice);

        // Set region as redraw area
        aDisplayInfo.SetRedrawArea(rRegion);

        // Writer or calc, coming from original RedrawOneLayer.
        aDisplayInfo.SetPagePainting(sal_False); // #i72889# no page painting for layer painting

        // paint page
        GetObjectContact().ProcessDisplay(aDisplayInfo);

        // delete PaintInfoRec
        delete pInfoRec;
    }

    // reset redirector
    GetObjectContact().SetViewObjectContactRedirector(0L);
}

// Invalidate call, used from ObjectContact(OfPageView) in InvalidatePartOfView(...)
void SdrPageWindow::Invalidate(const Rectangle& rRectangle)
{
    if(GetPageView().IsVisible() && GetPaintWindow().OutputToWindow())
    {
        ((Window&)GetPaintWindow().GetOutputDevice()).Invalidate(rRectangle, INVALIDATE_NOERASE);
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
        mpObjectContact->PrepareDelete();
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
