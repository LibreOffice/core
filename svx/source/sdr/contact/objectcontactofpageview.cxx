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

#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdr/contact/viewobjectcontactofunocontrol.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdview.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/animation/objectanimator.hxx>
#include <svx/sdr/event/eventhandler.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <svx/unoapi.hxx>



using namespace com::sun::star;



namespace sdr
{
    namespace contact
    {
        
        SdrPage* ObjectContactOfPageView::GetSdrPage() const
        {
            return GetPageWindow().GetPageView().GetPage();
        }

        ObjectContactOfPageView::ObjectContactOfPageView(SdrPageWindow& rPageWindow)
        :   ObjectContact(),
            mrPageWindow(rPageWindow)
        {
            
            setPreviewRenderer(((SdrPaintView&)rPageWindow.GetPageView().GetView()).IsPreviewRenderer());

            
            SetTimeout(1);
            Stop();
        }

        ObjectContactOfPageView::~ObjectContactOfPageView()
        {
            
            Timeout();
        }

        
        void ObjectContactOfPageView::setLazyInvalidate(ViewObjectContact& /*rVOC*/)
        {
            
            
            Start();
        }

        
        void ObjectContactOfPageView::PrepareProcessDisplay()
        {
            if(IsActive())
            {
                static bool bInvalidateDuringPaint(true);

                if(bInvalidateDuringPaint)
                {
                    
                    Timeout();
                }
            }
        }

        
        void ObjectContactOfPageView::Timeout()
        {
            
            Stop();

            
            const sal_uInt32 nVOCCount(getViewObjectContactCount());

            for(sal_uInt32 a(0); a < nVOCCount; a++)
            {
                ViewObjectContact* pCandidate = getViewObjectContact(a);
                pCandidate->triggerLazyInvalidate();
            }
        }

        
        void ObjectContactOfPageView::ProcessDisplay(DisplayInfo& rDisplayInfo)
        {
            const SdrPage* pStartPage = GetSdrPage();

            if(pStartPage && !rDisplayInfo.GetProcessLayers().IsEmpty())
            {
                const ViewContact& rDrawPageVC = pStartPage->GetViewContact();

                if(rDrawPageVC.GetObjectCount())
                {
                    DoProcessDisplay(rDisplayInfo);
                }
            }

            
            
            
            if(HasEventHandler())
            {
                sdr::event::TimerEventHandler& rEventHandler = GetEventHandler();

                if(!rEventHandler.IsEmpty())
                {
                    rEventHandler.Restart();
                }
            }
        }

        
        
        void ObjectContactOfPageView::DoProcessDisplay(DisplayInfo& rDisplayInfo)
        {
            
            
            bool bVisualizeEnteredGroup(DoVisualizeEnteredGroup() && !isOutputToPrinter());

            
            
            if(bVisualizeEnteredGroup)
            {
                rDisplayInfo.SetGhostedDrawMode();
            }

            
            OutputDevice* pOutDev = TryToGetOutputDevice();
            OSL_ENSURE(0 != pOutDev, "ObjectContactOfPageView without OutDev, someone has overloaded TryToGetOutputDevice wrong (!)");
            bool bClipRegionPushed(false);
            const Region& rRedrawArea(rDisplayInfo.GetRedrawArea());

            if(!rRedrawArea.IsEmpty())
            {
                bClipRegionPushed = true;
                pOutDev->Push(PUSH_CLIPREGION);
                pOutDev->IntersectClipRegion(rRedrawArea);
            }

            
            const ViewObjectContact& rDrawPageVOContact = GetSdrPage()->GetViewContact().GetViewObjectContact(*this);

            
            const double fCurrentTime(getPrimitiveAnimator().GetTime());
            OutputDevice& rTargetOutDev = GetPageWindow().GetPaintWindow().GetTargetOutputDevice();
            basegfx::B2DRange aViewRange;

            
            if(isOutputToRecordingMetaFile())
            {
                if(isOutputToPDFFile() || isOutputToPrinter())
                {
                    
                    
                    
                    
                    const Rectangle aLogicClipRectangle(rDisplayInfo.GetRedrawArea().GetBoundRect());

                    aViewRange = basegfx::B2DRange(
                        aLogicClipRectangle.Left(), aLogicClipRectangle.Top(),
                        aLogicClipRectangle.Right(), aLogicClipRectangle.Bottom());
                }
            }
            else
            {
                
                const Size aOutputSizePixel(rTargetOutDev.GetOutputSizePixel());
                aViewRange = basegfx::B2DRange(0.0, 0.0, aOutputSizePixel.getWidth(), aOutputSizePixel.getHeight());

                
                if(!rDisplayInfo.GetRedrawArea().IsEmpty())
                {
                    
                    const Rectangle aLogicClipRectangle(rDisplayInfo.GetRedrawArea().GetBoundRect());
                    basegfx::B2DRange aLogicClipRange(
                        aLogicClipRectangle.Left(), aLogicClipRectangle.Top(),
                        aLogicClipRectangle.Right(), aLogicClipRectangle.Bottom());
                    basegfx::B2DRange aDiscreteClipRange(aLogicClipRange);
                    aDiscreteClipRange.transform(rTargetOutDev.GetViewTransformation());

                    
                    
                    aDiscreteClipRange.expand(basegfx::B2DTuple(
                        floor(aDiscreteClipRange.getMinX()),
                        floor(aDiscreteClipRange.getMinY())));
                    aDiscreteClipRange.expand(basegfx::B2DTuple(
                        1.0 + ceil(aDiscreteClipRange.getMaxX()),
                        1.0 + ceil(aDiscreteClipRange.getMaxY())));

                    
                    aViewRange.intersect(aDiscreteClipRange);
                }

                
                aViewRange.transform(rTargetOutDev.GetInverseViewTransformation());
            }

            
            const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D(
                basegfx::B2DHomMatrix(),
                rTargetOutDev.GetViewTransformation(),
                aViewRange,
                GetXDrawPageForSdrPage(GetSdrPage()),
                fCurrentTime,
                uno::Sequence<beans::PropertyValue>());
            updateViewInformation2D(aNewViewInformation2D);

            
            
            drawinglayer::primitive2d::Primitive2DSequence xPrimitiveSequence(rDrawPageVOContact.getPrimitive2DSequenceHierarchy(rDisplayInfo));

            
            
            
            
            
            if(xPrimitiveSequence.hasElements())
            {
                
                rDisplayInfo.ClearGhostedDrawMode(); 
                pOutDev->SetLayoutMode(0); 

                
                drawinglayer::processor2d::BaseProcessor2D* pProcessor2D =
                    drawinglayer::processor2d::createProcessor2DFromOutputDevice(
                        rTargetOutDev, getViewInformation2D());

                if(pProcessor2D)
                {
                    pProcessor2D->process(xPrimitiveSequence);
                    delete pProcessor2D;
                }
            }

            
            if(bClipRegionPushed)
            {
                pOutDev->Pop();
            }

            
            if(bVisualizeEnteredGroup)
            {
                rDisplayInfo.ClearGhostedDrawMode();
            }
        }

        
        bool ObjectContactOfPageView::DoVisualizeEnteredGroup() const
        {
            SdrView& rView = GetPageWindow().GetPageView().GetView();
            return rView.DoVisualizeEnteredGroup();
        }

        
        const ViewContact* ObjectContactOfPageView::getActiveViewContact() const
        {
            SdrObjList* pActiveGroupList = GetPageWindow().GetPageView().GetObjList();

            if(pActiveGroupList)
            {
                if(pActiveGroupList->ISA(SdrPage))
                {
                    
                    return &(((SdrPage*)pActiveGroupList)->GetViewContact());
                }
                else if(pActiveGroupList->GetOwnerObj())
                {
                    
                    return &(pActiveGroupList->GetOwnerObj()->GetViewContact());
                }
            }
            else if(GetSdrPage())
            {
                
                return &(GetSdrPage()->GetViewContact());
            }

            return 0;
        }

        
        
        void ObjectContactOfPageView::InvalidatePartOfView(const basegfx::B2DRange& rRange) const
        {
            
            GetPageWindow().InvalidatePageWindow(rRange);
        }

        
        bool ObjectContactOfPageView::IsAreaVisible(const basegfx::B2DRange& rRange) const
        {
            
            if(rRange.isEmpty())
            {
                
                return false;
            }
            else
            {
                const OutputDevice& rTargetOutDev = GetPageWindow().GetPaintWindow().GetTargetOutputDevice();
                const Size aOutputSizePixel(rTargetOutDev.GetOutputSizePixel());
                basegfx::B2DRange aLogicViewRange(0.0, 0.0, aOutputSizePixel.getWidth(), aOutputSizePixel.getHeight());

                aLogicViewRange.transform(rTargetOutDev.GetInverseViewTransformation());

                if(!aLogicViewRange.isEmpty() && !aLogicViewRange.overlaps(rRange))
                {
                    return false;
                }
            }

            
            return ObjectContact::IsAreaVisible(rRange);
        }

        
        bool ObjectContactOfPageView::AreGluePointsVisible() const
        {
            return GetPageWindow().GetPageView().GetView().ImpIsGlueVisible();
        }

        
        bool ObjectContactOfPageView::IsTextAnimationAllowed() const
        {
            SdrView& rView = GetPageWindow().GetPageView().GetView();
            const SvtAccessibilityOptions& rOpt = rView.getAccessibilityOptions();
            return rOpt.GetIsAllowAnimatedText();
        }

        
        bool ObjectContactOfPageView::IsGraphicAnimationAllowed() const
        {
            SdrView& rView = GetPageWindow().GetPageView().GetView();
            const SvtAccessibilityOptions& rOpt = rView.getAccessibilityOptions();
            return rOpt.GetIsAllowAnimatedGraphics();
        }

        
        bool ObjectContactOfPageView::IsAsynchronGraphicsLoadingAllowed() const
        {
            SdrView& rView = GetPageWindow().GetPageView().GetView();
            return rView.IsSwapAsynchron();
        }

        
        bool ObjectContactOfPageView::IsMasterPageBufferingAllowed() const
        {
            SdrView& rView = GetPageWindow().GetPageView().GetView();
            return rView.IsMasterPagePaintCaching();
        }

        
        bool ObjectContactOfPageView::isOutputToPrinter() const
        {
            return (OUTDEV_PRINTER == mrPageWindow.GetPaintWindow().GetOutputDevice().GetOutDevType());
        }

        
        bool ObjectContactOfPageView::isOutputToWindow() const
        {
            return (OUTDEV_WINDOW == mrPageWindow.GetPaintWindow().GetOutputDevice().GetOutDevType());
        }

        
        bool ObjectContactOfPageView::isOutputToVirtualDevice() const
        {
            return (OUTDEV_VIRDEV == mrPageWindow.GetPaintWindow().GetOutputDevice().GetOutDevType());
        }

        
        bool ObjectContactOfPageView::isOutputToRecordingMetaFile() const
        {
            GDIMetaFile* pMetaFile = mrPageWindow.GetPaintWindow().GetOutputDevice().GetConnectMetaFile();
            return (pMetaFile && pMetaFile->IsRecord() && !pMetaFile->IsPause());
        }

        
        bool ObjectContactOfPageView::isOutputToPDFFile() const
        {
            return (0 != mrPageWindow.GetPaintWindow().GetOutputDevice().GetPDFWriter());
        }

        
        bool ObjectContactOfPageView::isDrawModeGray() const
        {
            const sal_uInt32 nDrawMode(mrPageWindow.GetPaintWindow().GetOutputDevice().GetDrawMode());
            return (nDrawMode == (DRAWMODE_GRAYLINE|DRAWMODE_GRAYFILL|DRAWMODE_BLACKTEXT|DRAWMODE_GRAYBITMAP|DRAWMODE_GRAYGRADIENT));
        }

        
        bool ObjectContactOfPageView::isDrawModeBlackWhite() const
        {
            const sal_uInt32 nDrawMode(mrPageWindow.GetPaintWindow().GetOutputDevice().GetDrawMode());
            return (nDrawMode == (DRAWMODE_BLACKLINE|DRAWMODE_BLACKTEXT|DRAWMODE_WHITEFILL|DRAWMODE_GRAYBITMAP|DRAWMODE_WHITEGRADIENT));
        }

        
        bool ObjectContactOfPageView::isDrawModeHighContrast() const
        {
            const sal_uInt32 nDrawMode(mrPageWindow.GetPaintWindow().GetOutputDevice().GetDrawMode());
            return (nDrawMode == (DRAWMODE_SETTINGSLINE|DRAWMODE_SETTINGSFILL|DRAWMODE_SETTINGSTEXT|DRAWMODE_SETTINGSGRADIENT));
        }

        
        SdrPageView* ObjectContactOfPageView::TryToGetSdrPageView() const
        {
            return &(mrPageWindow.GetPageView());
        }


        
        OutputDevice* ObjectContactOfPageView::TryToGetOutputDevice() const
        {
            SdrPreRenderDevice* pPreRenderDevice = mrPageWindow.GetPaintWindow().GetPreRenderDevice();

            if(pPreRenderDevice)
            {
                return &(pPreRenderDevice->GetPreRenderDevice());
            }
            else
            {
                return &(mrPageWindow.GetPaintWindow().GetOutputDevice());
            }
        }

        
        void ObjectContactOfPageView::SetUNOControlsDesignMode( bool _bDesignMode ) const
        {
            const sal_uInt32 nCount(getViewObjectContactCount());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                const ViewObjectContact* pVOC = getViewObjectContact(a);
                const ViewObjectContactOfUnoControl* pUnoObjectVOC = dynamic_cast< const ViewObjectContactOfUnoControl* >(pVOC);

                if(pUnoObjectVOC)
                {
                    pUnoObjectVOC->setControlDesignMode(_bDesignMode);
                }
            }
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
