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

#include <vcl/idle.hxx>
#include <svx/sdr/contact/viewobjectcontactofpageobj.hxx>
#include <svx/sdr/contact/viewcontactofpageobj.hxx>
#include <svx/svdopage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svtools/colorcfg.hxx>
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

namespace sdr { namespace contact {

class PagePrimitiveExtractor : public ObjectContactOfPagePainter, public Idle
{
private:
    // the ViewObjectContactOfPageObj using this painter
    ViewObjectContactOfPageObj&         mrViewObjectContactOfPageObj;

public:
    // basic constructor/destructor
    explicit PagePrimitiveExtractor(ViewObjectContactOfPageObj& rVOC);
    virtual ~PagePrimitiveExtractor() override;

    // LazyInvalidate request. Supported here to not automatically
    // invalidate the second interaction state all the time at the
    // original OC
    virtual void setLazyInvalidate(ViewObjectContact& rVOC) override;

    // From baseclass Timer, the timeout call triggered by the LazyInvalidate mechanism
    virtual void Invoke() final override;

    // get primitive visualization
    drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequenceForPage(const DisplayInfo& rDisplayInfo);

    // Own reaction on changes which will be forwarded to the OC of the owner-VOC
    virtual void InvalidatePartOfView(const basegfx::B2DRange& rRange) const override;

    // forward access to SdrPageView of ViewObjectContactOfPageObj
    virtual bool isOutputToPrinter() const override;
    virtual bool isOutputToWindow() const override;
    virtual bool isOutputToVirtualDevice() const override;
    virtual bool isOutputToRecordingMetaFile() const override;
    virtual bool isOutputToPDFFile() const override;
    virtual bool isDrawModeGray() const override;
    virtual bool isDrawModeBlackWhite() const override;
    virtual bool isDrawModeHighContrast() const override;
    virtual SdrPageView* TryToGetSdrPageView() const override;
    virtual OutputDevice* TryToGetOutputDevice() const override;
};

PagePrimitiveExtractor::PagePrimitiveExtractor(
    ViewObjectContactOfPageObj& rVOC)
:   ObjectContactOfPagePainter(rVOC.GetObjectContact()),
    mrViewObjectContactOfPageObj(rVOC)
{
    // make this renderer a preview renderer
    setPreviewRenderer(true);

    // init timer
    SetPriority(SchedulerPriority::HIGH_IDLE);
    Stop();
}

PagePrimitiveExtractor::~PagePrimitiveExtractor()
{
    // execute missing LazyInvalidates and stop timer
    Invoke();
}

void PagePrimitiveExtractor::setLazyInvalidate(ViewObjectContact& /*rVOC*/)
{
    // do NOT call parent, but remember that something is to do by
    // starting the LazyInvalidateTimer
    Start();
}

// From baseclass Timer, the timeout call triggered by the LazyInvalidate mechanism
void PagePrimitiveExtractor::Invoke()
{
    // stop the timer
    Stop();

    // invalidate all LazyInvalidate VOCs new situations
    const sal_uInt32 nVOCCount(getViewObjectContactCount());

    for(sal_uInt32 a(0); a < nVOCCount; a++)
    {
        ViewObjectContact* pCandidate = getViewObjectContact(a);
        pCandidate->triggerLazyInvalidate();
    }
}

drawinglayer::primitive2d::Primitive2DContainer PagePrimitiveExtractor::createPrimitive2DSequenceForPage(const DisplayInfo& /*rDisplayInfo*/)
{
    drawinglayer::primitive2d::Primitive2DContainer xRetval;
    SdrPage* pStartPage = GetStartPage();

    if(pStartPage)
    {
        // update own ViewInformation2D for visualized page
        const drawinglayer::geometry::ViewInformation2D& rOriginalViewInformation = mrViewObjectContactOfPageObj.GetObjectContact().getViewInformation2D();
        const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D(
            rOriginalViewInformation.getObjectTransformation(),
            rOriginalViewInformation.getViewTransformation(),

            // #i101075# use empty range for page content here to force
            // the content not to be physically clipped in any way. This
            // would be possible, but would require the internal transformation
            // which maps between the page visualisation object and the page
            // content, including the aspect ratios (for details see in
            // PagePreviewPrimitive2D::create2DDecomposition)
            basegfx::B2DRange(),

            GetXDrawPageForSdrPage(pStartPage),
            0.0, // no time; page previews are not animated
            rOriginalViewInformation.getExtendedInformationSequence());
        updateViewInformation2D(aNewViewInformation2D);

        // create copy of DisplayInfo to set PagePainting
        DisplayInfo aDisplayInfo;

        // get page's VOC
        ViewObjectContact& rDrawPageVOContact = pStartPage->GetViewContact().GetViewObjectContact(*this);

        // get whole Primitive2DContainer
        xRetval = rDrawPageVOContact.getPrimitive2DSequenceHierarchy(aDisplayInfo);
    }

    return xRetval;
}

void PagePrimitiveExtractor::InvalidatePartOfView(const basegfx::B2DRange& rRange) const
{
    // an invalidate is called at this view, this needs to be translated to an invalidate
    // for the using VOC. Coordinates are in page coordinate system.
    const SdrPage* pStartPage = GetStartPage();

    if(pStartPage && !rRange.isEmpty())
    {
        const basegfx::B2DRange aPageRange(0.0, 0.0, (double)pStartPage->GetWdt(), (double)pStartPage->GetHgt());

        if(rRange.overlaps(aPageRange))
        {
            // if object on the page is inside or overlapping with page, create ActionChanged() for
            // involved VOC
            mrViewObjectContactOfPageObj.ActionChanged();
        }
    }
}

// forward access to SdrPageView to VOCOfPageObj
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

drawinglayer::primitive2d::Primitive2DContainer ViewObjectContactOfPageObj::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
{
    drawinglayer::primitive2d::Primitive2DContainer xRetval;
    const SdrPageObj& rPageObject((static_cast< ViewContactOfPageObj& >(GetViewContact())).GetPageObj());
    const SdrPage* pPage = rPageObject.GetReferencedPage();
    const svtools::ColorConfig aColorConfig;

    // get PageObject's geometry
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

    // #i102637# add gray frame also when printing and page exists (handout pages)
    const bool bCreateGrayFrame(!GetObjectContact().isOutputToPrinter() || pPage);

    // get displayed page's content. This is the unscaled page content
    if(mpExtractor && pPage)
    {
        // get displayed page's geometry
           drawinglayer::primitive2d::Primitive2DContainer xPageContent;
        const Size aPageSize(pPage->GetSize());
        const double fPageWidth(aPageSize.getWidth());
        const double fPageHeight(aPageSize.getHeight());

        // The case that a PageObject contains another PageObject which visualizes the
        // same page again would lead to a recursion. Limit that recursion depth to one
        // by using a local static bool
        static bool bInCreatePrimitive2D(false);

        if(bInCreatePrimitive2D)
        {
            // Recursion is possible. Create a replacement primitive
            xPageContent.resize(2);
            const Color aDocColor(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);
            svtools::ColorConfigValue aBorderConfig = aColorConfig.GetColorValue(svtools::DOCBOUNDARIES);
            const Color aBorderColor = aBorderConfig.bIsVisible ? aBorderConfig.nColor : aDocColor;
            const basegfx::B2DRange aPageBound(0.0, 0.0, fPageWidth, fPageHeight);
            const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aPageBound));

            // add replacement fill
            xPageContent[0L] = drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aOutline), aDocColor.getBColor()));

            // add replacement border
            xPageContent[1L] = drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOutline, aBorderColor.getBColor()));
        }
        else
        {
            // set recursion flag
            bInCreatePrimitive2D = true;

            // init extractor, guarantee existence, set page there
            mpExtractor->SetStartPage(pPage);

            // #i105548# also need to copy the VOCRedirector for sub-content creation
            mpExtractor->SetViewObjectContactRedirector(GetObjectContact().GetViewObjectContactRedirector());

            // create page content
            xPageContent = mpExtractor->createPrimitive2DSequenceForPage(rDisplayInfo);

            // #i105548# reset VOCRedirector to not accidentally have a pointer to a
            // temporary class, so calls to it are avoided safely
            mpExtractor->SetViewObjectContactRedirector(nullptr);

            // reset recursion flag
            bInCreatePrimitive2D = false;
        }

        // prepare retval
        if(!xPageContent.empty())
        {
            const uno::Reference< drawing::XDrawPage > xDrawPage(GetXDrawPageForSdrPage(const_cast< SdrPage*>(pPage)));
            const drawinglayer::primitive2d::Primitive2DReference xPagePreview(new drawinglayer::primitive2d::PagePreviewPrimitive2D(
                xDrawPage, aPageObjectTransform, fPageWidth, fPageHeight, xPageContent));
            xRetval = drawinglayer::primitive2d::Primitive2DContainer { xPagePreview };
        }
    }
    else if(bCreateGrayFrame)
    {
        // #i105146# no content, but frame display. To make hitting the page preview objects
        // on the handout page more simple, add hidden fill geometry
        const drawinglayer::primitive2d::Primitive2DReference xFrameHit(
            drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                aPageObjectTransform));
        xRetval = drawinglayer::primitive2d::Primitive2DContainer { xFrameHit };
    }

    // add a gray outline frame, except not when printing
    if(bCreateGrayFrame)
    {
        const Color aFrameColor(aColorConfig.GetColorValue(svtools::OBJECTBOUNDARIES).nColor);
        basegfx::B2DPolygon aOwnOutline(basegfx::tools::createUnitPolygon());
        aOwnOutline.transform(aPageObjectTransform);

        const drawinglayer::primitive2d::Primitive2DReference xGrayFrame(
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOwnOutline, aFrameColor.getBColor()));

        xRetval.push_back(xGrayFrame);
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
    // delete the helper OC
    if(mpExtractor)
    {
        // remember candidate and reset own pointer to avoid action when createPrimitive2DSequence()
        // would be called for any reason
        PagePrimitiveExtractor* pCandidate = mpExtractor;
        mpExtractor = nullptr;

        // also reset the StartPage to avoid ActionChanged() forwardings in the
        // PagePrimitiveExtractor::InvalidatePartOfView() implementation
        pCandidate->SetStartPage(nullptr);
        delete pCandidate;
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
