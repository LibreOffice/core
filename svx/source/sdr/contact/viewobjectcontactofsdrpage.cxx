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


#include <sdr/contact/viewobjectcontactofsdrpage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <sdr/contact/viewcontactofsdrpage.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/gridprimitive2d.hxx>
#include <drawinglayer/primitive2d/helplineprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <sdr/primitive2d/sdrprimitivetools.hxx>

using namespace com::sun::star;

namespace sdr::contact {

const SdrPage& ViewObjectContactOfPageSubObject::getPage() const
{
    return static_cast< ViewContactOfPageSubObject& >(GetViewContact()).getPage();
}

ViewObjectContactOfPageSubObject::ViewObjectContactOfPageSubObject(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   ViewObjectContact(rObjectContact, rViewContact)
{
}

ViewObjectContactOfPageSubObject::~ViewObjectContactOfPageSubObject()
{
}

bool ViewObjectContactOfPageSubObject::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
{
    if(rDisplayInfo.GetSubContentActive())
    {
        return false;
    }

    if(rDisplayInfo.GetControlLayerProcessingActive())
    {
        return false;
    }

    if(!GetObjectContact().isPageDecorationActive())
    {
        return false;
    }

    if(GetObjectContact().isOutputToPrinter())
    {
        return false;
    }

    if(!GetObjectContact().TryToGetSdrPageView())
    {
        return false;
    }

    return true;
}

bool ViewObjectContactOfPageSubObject::isPrimitiveGhosted(const DisplayInfo& /*rDisplayInfo*/) const
{
    // suppress ghosted for page parts
    return false;
}

ViewObjectContactOfPageBackground::ViewObjectContactOfPageBackground(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   ViewObjectContactOfPageSubObject(rObjectContact, rViewContact)
{
}

ViewObjectContactOfPageBackground::~ViewObjectContactOfPageBackground()
{
}

bool ViewObjectContactOfPageBackground::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
{
    if(!ViewObjectContactOfPageSubObject::isPrimitiveVisible(rDisplayInfo))
    {
        return false;
    }

    // no page background for preview renderers
    if(GetObjectContact().IsPreviewRenderer())
    {
        return false;
    }

    return true;
}

void ViewObjectContactOfPageBackground::createPrimitive2DSequence(const DisplayInfo& /*rDisplayInfo*/, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
{
    // Initialize background. Dependent of IsPageVisible, use ApplicationBackgroundColor or ApplicationDocumentColor. Most
    // old renderers for export (html, pdf, gallery, ...) set the page to not visible (SetPageVisible(false)). They expect the
    // given OutputDevice to be initialized with the ApplicationDocumentColor then.
    const SdrPageView* pPageView = GetObjectContact().TryToGetSdrPageView();

    if(pPageView)
    {
        const SdrView& rView = pPageView->GetView();
        Color aInitColor;

        if(rView.IsPageVisible())
        {
            aInitColor = pPageView->GetApplicationBackgroundColor();
        }
        else
        {
            aInitColor = pPageView->GetApplicationDocumentColor();

            if(COL_AUTO == aInitColor)
            {
                const svtools::ColorConfig aColorConfig;
                aInitColor = aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor;
            }
        }

        // init background with InitColor
        const basegfx::BColor aRGBColor(aInitColor.getBColor());
        rVisitor.visit(new drawinglayer::primitive2d::BackgroundColorPrimitive2D(aRGBColor, (255 - aInitColor.GetAlpha()) / 255.0));
    }
}

ViewObjectContactOfMasterPage::ViewObjectContactOfMasterPage(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   ViewObjectContactOfPageSubObject(rObjectContact, rViewContact)
{
}

ViewObjectContactOfMasterPage::~ViewObjectContactOfMasterPage()
{
}

bool ViewObjectContactOfMasterPage::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
{
    if(!ViewObjectContactOfPageSubObject::isPrimitiveVisible(rDisplayInfo))
    {
        return false;
    }

    // this object is only used for MasterPages. When not the MasterPage is
    // displayed as a page, but another page is using it as sub-object, the
    // geometry needs to be hidden
    if(rDisplayInfo.GetSubContentActive())
    {
        return false;
    }

    return true;
}

ViewObjectContactOfPageFill::ViewObjectContactOfPageFill(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   ViewObjectContactOfPageSubObject(rObjectContact, rViewContact)
{
}

ViewObjectContactOfPageFill::~ViewObjectContactOfPageFill()
{
}

bool ViewObjectContactOfPageFill::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
{
    if(!ViewObjectContactOfPageSubObject::isPrimitiveVisible(rDisplayInfo))
    {
        return false;
    }

    SdrPageView* pSdrPageView = GetObjectContact().TryToGetSdrPageView();

    if(!pSdrPageView)
    {
        return false;
    }

    if(!pSdrPageView->GetView().IsPageVisible())
    {
        return false;
    }

    return true;
}

void ViewObjectContactOfPageFill::createPrimitive2DSequence(const DisplayInfo& /*rDisplayInfo*/, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
{
    const SdrPageView* pPageView = GetObjectContact().TryToGetSdrPageView();

    if(pPageView)
    {
        const SdrPage& rPage = getPage();

        const basegfx::B2DRange aPageFillRange(0.0, 0.0, static_cast<double>(rPage.GetWidth()), static_cast<double>(rPage.GetHeight()));
        const basegfx::B2DPolygon aPageFillPolygon(basegfx::utils::createPolygonFromRect(aPageFillRange));
        Color aPageFillColor;

        if(pPageView->GetApplicationDocumentColor() != COL_AUTO)
        {
            aPageFillColor = pPageView->GetApplicationDocumentColor();
        }
        else
        {
            const svtools::ColorConfig aColorConfig;
            aPageFillColor = aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor;
        }

        // create and add primitive
        const basegfx::BColor aRGBColor(aPageFillColor.getBColor());
        rVisitor.visit(new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPageFillPolygon), aRGBColor));
    }
}

ViewObjectContactOfPageShadow::ViewObjectContactOfPageShadow(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   ViewObjectContactOfPageSubObject(rObjectContact, rViewContact)
{
}

ViewObjectContactOfPageShadow::~ViewObjectContactOfPageShadow()
{
}

bool ViewObjectContactOfPageShadow::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
{
    if(!ViewObjectContactOfPageSubObject::isPrimitiveVisible(rDisplayInfo))
    {
        return false;
    }

    SdrPageView* pSdrPageView = GetObjectContact().TryToGetSdrPageView();

    if(!pSdrPageView)
    {
        return false;
    }

    if(!pSdrPageView->GetView().IsPageVisible())
    {
        return false;
    }

    if(!pSdrPageView->GetView().IsPageShadowVisible())
    {
        return false;
    }

    // no page shadow for preview renderers
    if(GetObjectContact().IsPreviewRenderer())
    {
        return false;
    }

    // no page shadow for high contrast mode
    if(GetObjectContact().isDrawModeHighContrast())
    {
        return false;
    }

    return true;
}

ViewObjectContactOfOuterPageBorder::ViewObjectContactOfOuterPageBorder(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   ViewObjectContactOfPageSubObject(rObjectContact, rViewContact)
{
}

ViewObjectContactOfOuterPageBorder::~ViewObjectContactOfOuterPageBorder()
{
}

bool ViewObjectContactOfOuterPageBorder::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
{
    if(!ViewObjectContactOfPageSubObject::isPrimitiveVisible(rDisplayInfo))
    {
        return false;
    }

    SdrPageView* pSdrPageView = GetObjectContact().TryToGetSdrPageView();

    if(!pSdrPageView)
    {
        return false;
    }

    const SdrView& rView = pSdrPageView->GetView();

    return rView.IsPageVisible() || !rView.IsPageBorderVisible();
}

ViewObjectContactOfInnerPageBorder::ViewObjectContactOfInnerPageBorder(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   ViewObjectContactOfPageSubObject(rObjectContact, rViewContact)
{
}

ViewObjectContactOfInnerPageBorder::~ViewObjectContactOfInnerPageBorder()
{
}

bool ViewObjectContactOfInnerPageBorder::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
{
    if(!ViewObjectContactOfPageSubObject::isPrimitiveVisible(rDisplayInfo))
    {
        return false;
    }

    SdrPageView* pSdrPageView = GetObjectContact().TryToGetSdrPageView();

    if(!pSdrPageView)
    {
        return false;
    }

    if(!pSdrPageView->GetView().IsBordVisible())
    {
        return false;
    }

    const SdrPage& rPage = getPage();

    if(!rPage.GetLeftBorder() && !rPage.GetUpperBorder() && !rPage.GetRightBorder() && !rPage.GetLowerBorder())
    {
        return false;
    }

    // no inner page border for preview renderers
    if(GetObjectContact().IsPreviewRenderer())
    {
        return false;
    }

    return true;
}

ViewObjectContactOfPageHierarchy::ViewObjectContactOfPageHierarchy(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   ViewObjectContactOfPageSubObject(rObjectContact, rViewContact)
{
}

ViewObjectContactOfPageHierarchy::~ViewObjectContactOfPageHierarchy()
{
}

void ViewObjectContactOfPageHierarchy::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
{
    // process local sub-hierarchy
    const sal_uInt32 nSubHierarchyCount(GetViewContact().GetObjectCount());

    if(!nSubHierarchyCount)
        return;

    getPrimitive2DSequenceSubHierarchy(rDisplayInfo, rVisitor);
}

ViewObjectContactOfPageGrid::ViewObjectContactOfPageGrid(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   ViewObjectContactOfPageSubObject(rObjectContact, rViewContact)
{
}

ViewObjectContactOfPageGrid::~ViewObjectContactOfPageGrid()
{
}

bool ViewObjectContactOfPageGrid::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
{
    if(!ViewObjectContactOfPageSubObject::isPrimitiveVisible(rDisplayInfo))
    {
        return false;
    }

    SdrPageView* pSdrPageView = GetObjectContact().TryToGetSdrPageView();

    if(!pSdrPageView)
    {
        return false;
    }

    const SdrView& rView = pSdrPageView->GetView();

    if(!rView.IsGridVisible())
    {
        return false;
    }

    // no page grid for preview renderers
    if(GetObjectContact().IsPreviewRenderer())
    {
        return false;
    }

    if(static_cast< ViewContactOfGrid& >(GetViewContact()).getFront() != rView.IsGridFront())
    {
        return false;
    }

    return true;
}

void ViewObjectContactOfPageGrid::createPrimitive2DSequence(const DisplayInfo& /*rDisplayInfo*/, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
{
    const SdrPageView* pPageView = GetObjectContact().TryToGetSdrPageView();

    if(pPageView)
    {
        const SdrView& rView = pPageView->GetView();
        const SdrPage& rPage = getPage();
        const Color aGridColor(rView.GetGridColor());
        const basegfx::BColor aRGBGridColor(aGridColor.getBColor());

        basegfx::B2DHomMatrix aGridMatrix;
        aGridMatrix.set(0, 0, static_cast<double>(rPage.GetWidth() - (rPage.GetRightBorder() + rPage.GetLeftBorder())));
        aGridMatrix.set(1, 1, static_cast<double>(rPage.GetHeight() - (rPage.GetLowerBorder() + rPage.GetUpperBorder())));
        aGridMatrix.set(0, 2, static_cast<double>(rPage.GetLeftBorder()));
        aGridMatrix.set(1, 2, static_cast<double>(rPage.GetUpperBorder()));

        const Size aRaw(rView.GetGridCoarse());
        const Size aFine(rView.GetGridFine());
        const double fWidthX(aRaw.getWidth());
        const double fWidthY(aRaw.getHeight());
        const sal_uInt32 nSubdivisionsX(aFine.getWidth() ? aRaw.getWidth() / aFine.getWidth() : 0);
        const sal_uInt32 nSubdivisionsY(aFine.getHeight() ? aRaw.getHeight() / aFine.getHeight() : 0);

        rVisitor.visit(new drawinglayer::primitive2d::GridPrimitive2D(
            aGridMatrix, fWidthX, fWidthY, 10.0, 3.0, nSubdivisionsX, nSubdivisionsY, aRGBGridColor,
            BitmapEx(drawinglayer::primitive2d::createDefaultCross_3x3(aRGBGridColor))));
    }
}

ViewObjectContactOfPageHelplines::ViewObjectContactOfPageHelplines(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   ViewObjectContactOfPageSubObject(rObjectContact, rViewContact)
{
}

ViewObjectContactOfPageHelplines::~ViewObjectContactOfPageHelplines()
{
}

bool ViewObjectContactOfPageHelplines::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
{
    if(!ViewObjectContactOfPageSubObject::isPrimitiveVisible(rDisplayInfo))
    {
        return false;
    }

    SdrPageView* pSdrPageView = GetObjectContact().TryToGetSdrPageView();

    if(!pSdrPageView)
    {
        return false;
    }

    const SdrView& rView = pSdrPageView->GetView();

    if(!rView.IsHlplVisible())
    {
        return false;
    }

    // no helplines for preview renderers
    if(GetObjectContact().IsPreviewRenderer())
    {
        return false;
    }

    if(static_cast< ViewContactOfHelplines& >(GetViewContact()).getFront() != rView.IsHlplFront())
    {
        return false;
    }

    return true;
}

void ViewObjectContactOfPageHelplines::createPrimitive2DSequence(const DisplayInfo& /*rDisplayInfo*/, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
{
    const SdrPageView* pPageView = GetObjectContact().TryToGetSdrPageView();

    if(pPageView)
    {
        const SdrHelpLineList& rHelpLineList = pPageView->GetHelpLines();
        const sal_uInt32 nCount(rHelpLineList.GetCount());

        if(nCount)
        {
            const basegfx::BColor aRGBColorA(1.0, 1.0, 1.0);
            const basegfx::BColor aRGBColorB(0.0, 0.0, 0.0);

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                const SdrHelpLine& rHelpLine = rHelpLineList[static_cast<sal_uInt16>(a)];
                const basegfx::B2DPoint aPosition(static_cast<double>(rHelpLine.GetPos().X()), static_cast<double>(rHelpLine.GetPos().Y()));
                const double fDiscreteDashLength(4.0);

                switch(rHelpLine.GetKind())
                {
                    default : // SdrHelpLineKind::Point
                    {
                        rVisitor.visit(new drawinglayer::primitive2d::HelplinePrimitive2D(
                            aPosition, basegfx::B2DVector(1.0, 0.0), drawinglayer::primitive2d::HelplineStyle2D::Point,
                            aRGBColorA, aRGBColorB, fDiscreteDashLength));
                        break;
                    }
                    case SdrHelpLineKind::Vertical :
                    {
                        rVisitor.visit(new drawinglayer::primitive2d::HelplinePrimitive2D(
                            aPosition, basegfx::B2DVector(0.0, 1.0), drawinglayer::primitive2d::HelplineStyle2D::Line,
                            aRGBColorA, aRGBColorB, fDiscreteDashLength));
                        break;
                    }
                    case SdrHelpLineKind::Horizontal :
                    {
                        rVisitor.visit(new drawinglayer::primitive2d::HelplinePrimitive2D(
                            aPosition, basegfx::B2DVector(1.0, 0.0), drawinglayer::primitive2d::HelplineStyle2D::Line,
                            aRGBColorA, aRGBColorB, fDiscreteDashLength));
                        break;
                    }
                }
            }
        }
    }
}

ViewObjectContactOfSdrPage::ViewObjectContactOfSdrPage(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   ViewObjectContact(rObjectContact, rViewContact)
{
}

ViewObjectContactOfSdrPage::~ViewObjectContactOfSdrPage()
{
}

void ViewObjectContactOfSdrPage::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
{
    // process local sub-hierarchy
    const sal_uInt32 nSubHierarchyCount(GetViewContact().GetObjectCount());

    if(!nSubHierarchyCount)
        return;

    const bool bDoGhostedDisplaying(
        GetObjectContact().DoVisualizeEnteredGroup()
        && !GetObjectContact().isOutputToPrinter()
        && GetObjectContact().getActiveViewContact() == &GetViewContact());

    if(bDoGhostedDisplaying)
    {
        rDisplayInfo.ClearGhostedDrawMode();
    }

    // visit object hierarchy
    getPrimitive2DSequenceSubHierarchy(rDisplayInfo, rVisitor);

    if(bDoGhostedDisplaying)
    {
        rDisplayInfo.SetGhostedDrawMode();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
