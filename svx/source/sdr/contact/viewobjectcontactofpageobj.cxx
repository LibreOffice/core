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


#include <svtools/colorcfg.hxx>

#include <svx/sdr/contact/viewobjectcontactofpageobj.hxx>
#include <svx/sdr/contact/viewcontactofpageobj.hxx>
#include <svx/svdopage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoapi.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>



using namespace com::sun::star;



namespace sdr
{
    namespace contact
    {
        class PagePrimitiveExtractor : public ObjectContactOfPagePainter, public Timer
        {
        private:
            
            ViewObjectContactOfPageObj&         mrViewObjectContactOfPageObj;

        public:
            
            explicit PagePrimitiveExtractor(ViewObjectContactOfPageObj& rVOC);
            virtual ~PagePrimitiveExtractor();

            
            
            
            virtual void setLazyInvalidate(ViewObjectContact& rVOC);

            
            virtual void Timeout();

            
            drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequenceForPage(const DisplayInfo& rDisplayInfo);

            
            virtual void InvalidatePartOfView(const basegfx::B2DRange& rRange) const;

            
            virtual bool isOutputToPrinter() const;
            virtual bool isOutputToWindow() const;
            virtual bool isOutputToVirtualDevice() const;
            virtual bool isOutputToRecordingMetaFile() const;
            virtual bool isOutputToPDFFile() const;
            virtual bool isDrawModeGray() const;
            virtual bool isDrawModeBlackWhite() const;
            virtual bool isDrawModeHighContrast() const;
            virtual SdrPageView* TryToGetSdrPageView() const;
            virtual OutputDevice* TryToGetOutputDevice() const;
        };

        PagePrimitiveExtractor::PagePrimitiveExtractor(
            ViewObjectContactOfPageObj& rVOC)
        :   ObjectContactOfPagePainter(0, rVOC.GetObjectContact()),
            mrViewObjectContactOfPageObj(rVOC)
        {
            
            setPreviewRenderer(true);

            
            SetTimeout(1);
            Stop();
        }

        PagePrimitiveExtractor::~PagePrimitiveExtractor()
        {
            
            Timeout();
        }

        void PagePrimitiveExtractor::setLazyInvalidate(ViewObjectContact& /*rVOC*/)
        {
            
            
            Start();
        }

        
        void PagePrimitiveExtractor::Timeout()
        {
            
            Stop();

            
            const sal_uInt32 nVOCCount(getViewObjectContactCount());

            for(sal_uInt32 a(0); a < nVOCCount; a++)
            {
                ViewObjectContact* pCandidate = getViewObjectContact(a);
                pCandidate->triggerLazyInvalidate();
            }
        }

        drawinglayer::primitive2d::Primitive2DSequence PagePrimitiveExtractor::createPrimitive2DSequenceForPage(const DisplayInfo& /*rDisplayInfo*/)
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SdrPage* pStartPage = GetStartPage();

            if(pStartPage)
            {
                
                const drawinglayer::geometry::ViewInformation2D& rOriginalViewInformation = mrViewObjectContactOfPageObj.GetObjectContact().getViewInformation2D();
                const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D(
                    rOriginalViewInformation.getObjectTransformation(),
                    rOriginalViewInformation.getViewTransformation(),

                    
                    
                    
                    
                    
                    
                    basegfx::B2DRange(),

                    GetXDrawPageForSdrPage(const_cast< SdrPage* >(pStartPage)),
                    0.0, 
                    rOriginalViewInformation.getExtendedInformationSequence());
                updateViewInformation2D(aNewViewInformation2D);

                
                DisplayInfo aDisplayInfo;

                
                ViewObjectContact& rDrawPageVOContact = pStartPage->GetViewContact().GetViewObjectContact(*this);

                
                xRetval = rDrawPageVOContact.getPrimitive2DSequenceHierarchy(aDisplayInfo);
            }

            return xRetval;
        }

        void PagePrimitiveExtractor::InvalidatePartOfView(const basegfx::B2DRange& rRange) const
        {
            
            
            const SdrPage* pStartPage = GetStartPage();

            if(pStartPage && !rRange.isEmpty())
            {
                const basegfx::B2DRange aPageRange(0.0, 0.0, (double)pStartPage->GetWdt(), (double)pStartPage->GetHgt());

                if(rRange.overlaps(aPageRange))
                {
                    
                    
                    mrViewObjectContactOfPageObj.ActionChanged();
                }
            }
        }

        
        bool PagePrimitiveExtractor::isOutputToPrinter() const { return mrViewObjectContactOfPageObj.GetObjectContact().isOutputToPrinter(); }
        bool PagePrimitiveExtractor::isOutputToWindow() const { return mrViewObjectContactOfPageObj.GetObjectContact().isOutputToWindow(); }
        bool PagePrimitiveExtractor::isOutputToVirtualDevice() const { return mrViewObjectContactOfPageObj.GetObjectContact().isOutputToVirtualDevice(); }
        bool PagePrimitiveExtractor::isOutputToRecordingMetaFile() const { return mrViewObjectContactOfPageObj.GetObjectContact().isOutputToRecordingMetaFile(); }
        bool PagePrimitiveExtractor::isOutputToPDFFile() const { return mrViewObjectContactOfPageObj.GetObjectContact().isOutputToPDFFile(); }
        bool PagePrimitiveExtractor::isDrawModeGray() const { return mrViewObjectContactOfPageObj.GetObjectContact().isDrawModeGray(); }
        bool PagePrimitiveExtractor::isDrawModeBlackWhite() const { return mrViewObjectContactOfPageObj.GetObjectContact().isDrawModeBlackWhite(); }
        bool PagePrimitiveExtractor::isDrawModeHighContrast() const { return mrViewObjectContactOfPageObj.GetObjectContact().isDrawModeHighContrast(); }
        SdrPageView* PagePrimitiveExtractor::TryToGetSdrPageView() const { return mrViewObjectContactOfPageObj.GetObjectContact().TryToGetSdrPageView(); }
        OutputDevice* PagePrimitiveExtractor::TryToGetOutputDevice() const { return mrViewObjectContactOfPageObj.GetObjectContact().TryToGetOutputDevice(); }
    } 
} 



namespace sdr
{
    namespace contact
    {
        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfPageObj::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SdrPageObj& rPageObject((static_cast< ViewContactOfPageObj& >(GetViewContact())).GetPageObj());
            const SdrPage* pPage = rPageObject.GetReferencedPage();
            const svtools::ColorConfig aColorConfig;

            
            basegfx::B2DHomMatrix aPageObjectTransform;
            {
                const Rectangle aPageObjectModelData(rPageObject.GetLastBoundRect());
                const basegfx::B2DRange aPageObjectBound(
                    aPageObjectModelData.Left(), aPageObjectModelData.Top(),
                    aPageObjectModelData.Right(), aPageObjectModelData.Bottom());

                aPageObjectTransform.set(0, 0, aPageObjectBound.getWidth());
                aPageObjectTransform.set(1, 1, aPageObjectBound.getHeight());
                aPageObjectTransform.set(0, 2, aPageObjectBound.getMinX());
                aPageObjectTransform.set(1, 2, aPageObjectBound.getMinY());
            }

            
            const bool bCreateGrayFrame(!GetObjectContact().isOutputToPrinter() || pPage);

            
            if(mpExtractor && pPage)
            {
                
                   drawinglayer::primitive2d::Primitive2DSequence xPageContent;
                const Size aPageSize(pPage->GetSize());
                const double fPageWidth(aPageSize.getWidth());
                const double fPageHeight(aPageSize.getHeight());

                
                
                
                static bool bInCreatePrimitive2D(false);

                if(bInCreatePrimitive2D)
                {
                    
                    xPageContent.realloc(2);
                    const Color aDocColor(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);
                    const Color aBorderColor(aColorConfig.GetColorValue(svtools::DOCBOUNDARIES).nColor);
                    const basegfx::B2DRange aPageBound(0.0, 0.0, fPageWidth, fPageHeight);
                    const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aPageBound));

                    
                    xPageContent[0L] = drawinglayer::primitive2d::Primitive2DReference(
                        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aOutline), aDocColor.getBColor()));

                    
                    xPageContent[1L] = drawinglayer::primitive2d::Primitive2DReference(
                        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOutline, aBorderColor.getBColor()));
                }
                else
                {
                    
                    bInCreatePrimitive2D = true;

                    
                    mpExtractor->SetStartPage(pPage);

                    
                    mpExtractor->SetViewObjectContactRedirector(GetObjectContact().GetViewObjectContactRedirector());

                    
                    xPageContent = mpExtractor->createPrimitive2DSequenceForPage(rDisplayInfo);

                    
                    
                    mpExtractor->SetViewObjectContactRedirector(0);

                    
                    bInCreatePrimitive2D = false;
                }

                
                if(xPageContent.hasElements())
                {
                    const uno::Reference< drawing::XDrawPage > xDrawPage(GetXDrawPageForSdrPage(const_cast< SdrPage*>(pPage)));
                    const drawinglayer::primitive2d::Primitive2DReference xPagePreview(new drawinglayer::primitive2d::PagePreviewPrimitive2D(
                        xDrawPage, aPageObjectTransform, fPageWidth, fPageHeight, xPageContent, true));
                    xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xPagePreview, 1);
                }
            }
            else if(bCreateGrayFrame)
            {
                
                
                const drawinglayer::primitive2d::Primitive2DReference xFrameHit(
                    drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                        false,
                        aPageObjectTransform));
                xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xFrameHit, 1);
            }

            
            if(bCreateGrayFrame)
            {
                const Color aFrameColor(aColorConfig.GetColorValue(svtools::OBJECTBOUNDARIES).nColor);
                basegfx::B2DPolygon aOwnOutline(basegfx::tools::createUnitPolygon());
                aOwnOutline.transform(aPageObjectTransform);

                const drawinglayer::primitive2d::Primitive2DReference xGrayFrame(
                    new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOwnOutline, aFrameColor.getBColor()));

                drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval, xGrayFrame);
            }

            return xRetval;
        }

        ViewObjectContactOfPageObj::ViewObjectContactOfPageObj(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact),
            mpExtractor(new PagePrimitiveExtractor(*this))
        {
        }

        ViewObjectContactOfPageObj::~ViewObjectContactOfPageObj()
        {
            
            if(mpExtractor)
            {
                
                
                PagePrimitiveExtractor* pCandidate = mpExtractor;
                mpExtractor = 0;

                
                
                pCandidate->SetStartPage(0);
                delete pCandidate;
            }
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
