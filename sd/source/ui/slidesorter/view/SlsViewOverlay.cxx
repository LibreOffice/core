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

#include "precompiled_sd.hxx"

#include "view/SlsViewOverlay.hxx"

#include "SlideSorter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "view/SlideSorterView.hxx"
#include "SlideSorterViewShell.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObject.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "UpdateLockManager.hxx"

#include "Window.hxx"
#include "sdpage.hxx"

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <vcl/svapp.hxx>

#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>

using namespace ::sdr::overlay;

namespace {
    const static sal_Int32 gnSubstitutionStripeLength (3);
}

namespace sd { namespace slidesorter { namespace view {

//=====  ViewOverlay  =========================================================

ViewOverlay::ViewOverlay (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      maSelectionRectangleOverlay(*this),
      maMouseOverIndicatorOverlay(*this),
      maInsertionIndicatorOverlay(*this),
      maSubstitutionOverlay(*this)
{
}




ViewOverlay::~ViewOverlay (void)
{
}




SelectionRectangleOverlay& ViewOverlay::GetSelectionRectangleOverlay (void)
{
    return maSelectionRectangleOverlay;
}




MouseOverIndicatorOverlay& ViewOverlay::GetMouseOverIndicatorOverlay (void)
{
    return maMouseOverIndicatorOverlay;
}




InsertionIndicatorOverlay& ViewOverlay::GetInsertionIndicatorOverlay (void)
{
    return maInsertionIndicatorOverlay;
}




SubstitutionOverlay& ViewOverlay::GetSubstitutionOverlay (void)
{
    return maSubstitutionOverlay;
}




SlideSorter& ViewOverlay::GetSlideSorter (void) const
{
    return mrSlideSorter;
}




OverlayManager* ViewOverlay::GetOverlayManager (void) const
{
    OverlayManager* pOverlayManager = NULL;

    SlideSorterView& rView (mrSlideSorter.GetView());
    SdrPageView* pPageView = rView.GetSdrPageView();
    if (pPageView != NULL && pPageView->PageWindowCount()>0)
    {
        SdrPageWindow* pPageWindow = pPageView->GetPageWindow(0);
        if (pPageWindow != NULL)
            pOverlayManager = pPageWindow->GetOverlayManager();
    }

    return pOverlayManager;
}




//=====  OverlayBase  =========================================================

OverlayBase::OverlayBase (ViewOverlay& rViewOverlay)
    : OverlayObject(Color(0,0,0)),
      mrViewOverlay(rViewOverlay)
{
    setVisible(false);
}




OverlayBase::~OverlayBase (void)
{
    OSL_ENSURE(!getOverlayManager(), "Please call RemoveRegistration() in the derived class; it's too late to call it in the base class since virtual methods will be missing when called in the destructor.");
}




void OverlayBase::EnsureRegistration (void)
{
    if (getOverlayManager() == NULL)
    {
        OverlayManager* pOverlayManager = mrViewOverlay.GetOverlayManager();
        if (pOverlayManager != NULL)
            pOverlayManager->add(*this);
    }
}




void OverlayBase::RemoveRegistration()
{
    OverlayManager* pOverlayManager = getOverlayManager();
    if (pOverlayManager != NULL)
        pOverlayManager->remove(*this);
}




//=====  SubstitutionOverlay  =================================================

SubstitutionOverlay::SubstitutionOverlay (ViewOverlay& rViewOverlay)
    : OverlayBase(rViewOverlay),
      maPosition(0,0),
      maShapes()
{
    allowAntiAliase(false);
}




SubstitutionOverlay::~SubstitutionOverlay (void)
{
    RemoveRegistration();
}




void SubstitutionOverlay::Create (
    model::PageEnumeration& rSelection,
    const Point& rPosition)
{
    EnsureRegistration();

    maPosition = rPosition;

    maShapes.clear();
    while (rSelection.HasMoreElements())
    {
        const Rectangle aBox (rSelection.GetNextElement()->GetPageObject()->GetCurrentBoundRect());
        basegfx::B2DRectangle aB2DBox(
            aBox.Left(),
            aBox.Top(),
            aBox.Right(),
            aBox.Bottom());
        maShapes.append(basegfx::tools::createPolygonFromRect(aB2DBox), 4);
    }

    setVisible(maShapes.count() > 0);
    // The selection indicator may have been visible already so call
    // objectChange() to enforce an update.
    objectChange();
}




void SubstitutionOverlay::Clear (void)
{
    maShapes.clear();
    setVisible(false);
}




void SubstitutionOverlay::Move (const Point& rOffset)
{
    const basegfx::B2DHomMatrix aTranslation(basegfx::tools::createTranslateB2DHomMatrix(rOffset.X(), rOffset.Y()));

    maShapes.transform(aTranslation);
    maPosition += rOffset;

    objectChange();
}




void SubstitutionOverlay::SetPosition (const Point& rPosition)
{
    Move(rPosition - GetPosition());
}




Point SubstitutionOverlay::GetPosition (void) const
{
    return maPosition;
}




drawinglayer::primitive2d::Primitive2DSequence SubstitutionOverlay::createOverlayObjectPrimitive2DSequence()
{
    drawinglayer::primitive2d::Primitive2DSequence aRetval;
    const sal_uInt32 nCount(maShapes.count());

    if(nCount && getOverlayManager())
    {
        aRetval.realloc(nCount);
        const basegfx::BColor aRGBColorA(getOverlayManager()->getStripeColorA().getBColor());
        const basegfx::BColor aRGBColorB(getOverlayManager()->getStripeColorB().getBColor());

        for(sal_uInt32 a(0); a < nCount; a++)
        {
            aRetval[a] = drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::PolygonMarkerPrimitive2D(
                    maShapes.getB2DPolygon(a),
                    aRGBColorA,
                    aRGBColorB,
                    gnSubstitutionStripeLength));
        }
    }

    return aRetval;
}

void SubstitutionOverlay::stripeDefinitionHasChanged()
{
    // react on OverlayManager's stripe definition change
    objectChange();
}


//=====  SelectionRectangleOverlay  ===========================================

SelectionRectangleOverlay::SelectionRectangleOverlay (ViewOverlay& rViewOverlay)
    : OverlayBase (rViewOverlay),
      maAnchor(0,0),
      maSecondCorner(0,0)
{
}



SelectionRectangleOverlay::~SelectionRectangleOverlay()
{
    RemoveRegistration();
}




Rectangle SelectionRectangleOverlay::GetSelectionRectangle (void)
{
    return Rectangle(maAnchor, maSecondCorner);
}




void SelectionRectangleOverlay::Start (const Point& rAnchor)
{
    EnsureRegistration();
    setVisible(false);
    maAnchor = rAnchor;
}




void SelectionRectangleOverlay::Update (const Point& rSecondCorner)
{
    maSecondCorner = rSecondCorner;
    setVisible(true);
    // The selection rectangle may have been visible already so call
    // objectChange() to enforce an update.
    objectChange();
}




drawinglayer::primitive2d::Primitive2DSequence SelectionRectangleOverlay::createOverlayObjectPrimitive2DSequence()
{
    drawinglayer::primitive2d::Primitive2DSequence aRetval;
    const basegfx::B2DRange aRange(maAnchor.X(), maAnchor.Y(), maSecondCorner.X(), maSecondCorner.Y());
    const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(aRange));

    if(aPolygon.count())
    {
        const basegfx::BColor aRGBColorA(getOverlayManager()->getStripeColorA().getBColor());
        const basegfx::BColor aRGBColorB(getOverlayManager()->getStripeColorB().getBColor());
        const drawinglayer::primitive2d::Primitive2DReference xReference(
            new drawinglayer::primitive2d::PolygonMarkerPrimitive2D(
                aPolygon,
                aRGBColorA,
                aRGBColorB,
                gnSubstitutionStripeLength));

        aRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
    }

    return aRetval;
}

void SelectionRectangleOverlay::stripeDefinitionHasChanged()
{
    // react on OverlayManager's stripe definition change
    objectChange();
}




//=====  InsertionIndicatorOverlay  ===========================================

InsertionIndicatorOverlay::InsertionIndicatorOverlay (ViewOverlay& rViewOverlay)
    : OverlayBase (rViewOverlay),
      mnInsertionIndex(-1),
      maBoundingBox()
{
}




InsertionIndicatorOverlay::~InsertionIndicatorOverlay()
{
    RemoveRegistration();
}




void InsertionIndicatorOverlay::SetPositionAndSize (const Rectangle& aNewBoundingBox)
{
    EnsureRegistration();
    maBoundingBox = aNewBoundingBox;
    setVisible( ! maBoundingBox.IsEmpty());
    // The insertion indicator may have been visible already so call
    // objectChange() to enforce an update.
    objectChange();
}




void InsertionIndicatorOverlay::SetPosition (const Point& rPoint)
{
    static const bool bAllowHorizontalInsertMarker = true;
    Layouter& rLayouter (mrViewOverlay.GetSlideSorter().GetView().GetLayouter());
    USHORT nPageCount
        = (USHORT)mrViewOverlay.GetSlideSorter().GetModel().GetPageCount();

    sal_Int32 nInsertionIndex = rLayouter.GetInsertionIndex (rPoint,
        bAllowHorizontalInsertMarker);
    if (nInsertionIndex >= nPageCount)
        nInsertionIndex = nPageCount-1;
    sal_Int32 nDrawIndex = nInsertionIndex;

    bool bVertical = false;
    bool bLeftOrTop = false;
    if (nInsertionIndex >= 0)
    {
        // Now that we know where to insert, we still have to determine
        // where to draw the marker.  There are two decisions to make:
        // 1. Draw a vertical or a horizontal insert marker.
        //    The horizontal one may only be chosen when there is only one
        //    column.
        // 2. The vertical (standard) insert marker may be painted left to
        //    the insert page or right of the previous one.  When both pages
        //    are in the same row this makes no difference.  Otherwise the
        //    posiotions are at the left and right ends of two rows.

        Point aPageCenter (rLayouter.GetPageObjectBox (
            nInsertionIndex).Center());

        if (bAllowHorizontalInsertMarker
            && rLayouter.GetColumnCount() == 1)
        {
            bVertical = false;
            bLeftOrTop = (rPoint.Y() <= aPageCenter.Y());
        }
        else
        {
            bVertical = true;
            bLeftOrTop = (rPoint.X() <= aPageCenter.X());
        }

        // Add one when the mark was painted below or to the right of the
        // page object.
        if ( ! bLeftOrTop)
            nInsertionIndex += 1;
    }

    mnInsertionIndex = nInsertionIndex;

    Rectangle aBox;
    if (mnInsertionIndex >= 0)
        aBox = rLayouter.GetInsertionMarkerBox (
            nDrawIndex,
            bVertical,
            bLeftOrTop);
    SetPositionAndSize (aBox);
}




sal_Int32 InsertionIndicatorOverlay::GetInsertionPageIndex (void) const
{
    return mnInsertionIndex;
}




drawinglayer::primitive2d::Primitive2DSequence InsertionIndicatorOverlay::createOverlayObjectPrimitive2DSequence()
{
    drawinglayer::primitive2d::Primitive2DSequence aRetval(2);
    const basegfx::B2DRange aRange(maBoundingBox.Left(), maBoundingBox.Top(), maBoundingBox.Right(), maBoundingBox.Bottom());
    const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(aRange));
    const basegfx::BColor aRGBColor(Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetFontColor().getBColor());

    aRetval[0] = drawinglayer::primitive2d::Primitive2DReference(
        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
            basegfx::B2DPolyPolygon(aPolygon),
            aRGBColor));
    aRetval[1] = drawinglayer::primitive2d::Primitive2DReference(
        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
            aPolygon,
            aRGBColor));

    return aRetval;
}




//=====  MouseOverIndicatorOverlay  ===========================================

MouseOverIndicatorOverlay::MouseOverIndicatorOverlay (ViewOverlay& rViewOverlay)
    : OverlayBase (rViewOverlay),
      mpPageUnderMouse()
{
}




MouseOverIndicatorOverlay::~MouseOverIndicatorOverlay (void)
{
    RemoveRegistration();
}




void MouseOverIndicatorOverlay::SetSlideUnderMouse (
    const model::SharedPageDescriptor& rpDescriptor)
{
    ViewShellBase* pBase = mrViewOverlay.GetSlideSorter().GetViewShellBase();
    if (pBase==NULL || ! pBase->GetUpdateLockManager()->IsLocked())
    {
        model::SharedPageDescriptor pDescriptor;
        if ( ! mpPageUnderMouse.expired())
        {
            try
            {
                pDescriptor = model::SharedPageDescriptor(mpPageUnderMouse);
            }
            catch (::boost::bad_weak_ptr)
            {
            }
        }

         if (pDescriptor != rpDescriptor)
        {
            // Switch to the new (possibly empty) descriptor.
            mpPageUnderMouse = rpDescriptor;

            EnsureRegistration();

            // Show the indicator when a valid page descriptor is given.
            setVisible( ! mpPageUnderMouse.expired());
            // The mouse over indicator may have been visible already so call
            // objectChange() to enforce an update.
            objectChange();
        }
    }
}




drawinglayer::primitive2d::Primitive2DSequence MouseOverIndicatorOverlay::createOverlayObjectPrimitive2DSequence()
{
    view::PageObjectViewObjectContact* pContact = GetViewObjectContact();

    if(pContact)
    {
        return pContact->createMouseOverEffectPrimitive2DSequence();
    }

    return drawinglayer::primitive2d::Primitive2DSequence();
}




view::PageObjectViewObjectContact* MouseOverIndicatorOverlay::GetViewObjectContact (void) const
{
    if ( ! mpPageUnderMouse.expired())
    {
        model::SharedPageDescriptor pDescriptor (mpPageUnderMouse);
        return pDescriptor->GetViewObjectContact();
    }
    return NULL;
}




} } } // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
