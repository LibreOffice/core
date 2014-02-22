/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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



using namespace ::rtl;
using namespace ::com::sun::star;



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
            
            uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            const_cast< SdrPageWindow* >( this )->mxControlContainer = uno::Reference< awt::XControlContainer >(xFactory->createInstance("com.sun.star.awt.UnoControlContainer"), uno::UNO_QUERY);
            uno::Reference< awt::XControlModel > xModel(xFactory->createInstance("com.sun.star.awt.UnoControlContainerModel"), uno::UNO_QUERY);
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
    
    ResetObjectContact();

    if (mxControlContainer.is())
    {
        SdrView& rView = GetPageView().GetView();

        
        FmFormView* pViewAsFormView = dynamic_cast< FmFormView* >( &rView );
        if ( pViewAsFormView )
            pViewAsFormView->RemoveControlContainer(mxControlContainer);

        
        uno::Reference< lang::XComponent > xComponent(mxControlContainer, uno::UNO_QUERY);
        xComponent->dispose();
    }
}


sdr::contact::ObjectContact* SdrPageWindow::CreateViewSpecificObjectContact()
{
    return new sdr::contact::ObjectContactOfPageView(*this);
}


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
    
    if(HasObjectContact())
    {
        GetObjectContact().PrepareProcessDisplay();
    }
}

void SdrPageWindow::PrepareRedraw(const Region& rReg)
{
    
    if(HasObjectContact())
    {
        GetObjectContact().PrepareProcessDisplay();
    }

    
    
    GetPaintWindow().SetRedrawRegion(rReg);
}



#ifdef CLIPPER_TEST
#include <svx/svdopath.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <tools/helpers.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>


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
} 
#endif 



void SdrPageWindow::RedrawAll(sdr::contact::ViewObjectContactRedirector* pRedirector) const
{
    
    GetObjectContact().SetViewObjectContactRedirector(pRedirector);

    
    const SdrView& rView = mrPageView.GetView();
    SdrModel& rModel = *((SdrModel*)rView.GetModel());

    
    const bool bPrinter(GetPaintWindow().OutputToPrinter());
    SetOfByte aProcessLayers = bPrinter ? mrPageView.GetPrintableLayers() : mrPageView.GetVisibleLayers();

    
    const Region& rRegion = GetPaintWindow().GetRedrawRegion();

    
    sdr::contact::DisplayInfo aDisplayInfo;

    
    
    const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
    const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), false);
    aProcessLayers.Clear(nControlLayerId);

    
    if(!aProcessLayers.IsEmpty())
    {
        aDisplayInfo.SetProcessLayers(aProcessLayers);

        
        aDisplayInfo.SetRedrawArea(rRegion);

        
        aDisplayInfo.SetPageProcessingActive(rView.IsPagePaintingAllowed()); 

        
        GetObjectContact().ProcessDisplay(aDisplayInfo);
    }

    
    GetObjectContact().SetViewObjectContactRedirector(0L);

    
#ifdef CLIPPER_TEST
    if(true)
    {
        impTryTest(GetPageView(), GetPaintWindow().GetOutputDevice());
    }
#endif 
}

void SdrPageWindow::RedrawLayer(const SdrLayerID* pId, sdr::contact::ViewObjectContactRedirector* pRedirector) const
{
    
    GetObjectContact().SetViewObjectContactRedirector(pRedirector);

    
    const SdrView& rView = mrPageView.GetView();
    SdrModel& rModel = *((SdrModel*)rView.GetModel());

    
    const bool bPrinter(GetPaintWindow().OutputToPrinter());
    SetOfByte aProcessLayers = bPrinter ? mrPageView.GetPrintableLayers() : mrPageView.GetVisibleLayers();

    
    if(aProcessLayers.IsSet(*pId))
    {
        
        const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
        const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), false);
        const bool bControlLayerProcessingActive(nControlLayerId == *pId);

        
        const Region& rRegion = GetPaintWindow().GetRedrawRegion();

        
        sdr::contact::DisplayInfo aDisplayInfo;

        
        aDisplayInfo.SetControlLayerProcessingActive(bControlLayerProcessingActive);

        
        aProcessLayers.ClearAll();
        aProcessLayers.Set(*pId);

        aDisplayInfo.SetProcessLayers(aProcessLayers);

        
        aDisplayInfo.SetRedrawArea(rRegion);

        
        
        aDisplayInfo.SetPageProcessingActive(false);

        
        GetObjectContact().ProcessDisplay(aDisplayInfo);
    }

    
    GetObjectContact().SetViewObjectContactRedirector(0L);
}


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
