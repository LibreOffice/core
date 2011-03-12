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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/viewobjectcontactofsdrpage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewcontactofsdrpage.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/gridprimitive2d.hxx>
#include <drawinglayer/primitive2d/helplineprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/sdr/primitive2d/sdrprimitivetools.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
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

            if(!rDisplayInfo.GetPageProcessingActive())
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
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
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

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfPageBackground::createPrimitive2DSequence(const DisplayInfo& /*rDisplayInfo*/) const
        {
            // Initialize background. Dependent of IsPageVisible, use ApplicationBackgroundColor or ApplicationDocumentColor. Most
            // old renderers for export (html, pdf, gallery, ...) set the page to not visible (SetPageVisible(false)). They expect the
            // given OutputDevice to be initialized with the ApplicationDocumentColor then.
            const SdrPageView* pPageView = GetObjectContact().TryToGetSdrPageView();
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

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

                    if(Color(COL_AUTO) == aInitColor)
                    {
                        const svtools::ColorConfig aColorConfig;
                        aInitColor = aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor;
                    }
                }

                // init background with InitColor
                xRetval.realloc(1);
                const basegfx::BColor aRGBColor(aInitColor.getBColor());
                xRetval[0] = drawinglayer::primitive2d::Primitive2DReference(new drawinglayer::primitive2d::BackgroundColorPrimitive2D(aRGBColor));
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
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
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
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

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfPageFill::createPrimitive2DSequence(const DisplayInfo& /*rDisplayInfo*/) const
        {
            const SdrPageView* pPageView = GetObjectContact().TryToGetSdrPageView();
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            if(pPageView)
            {
                const SdrPage& rPage = getPage();

                const basegfx::B2DRange aPageFillRange(0.0, 0.0, (double)rPage.GetWdt(), (double)rPage.GetHgt());
                const basegfx::B2DPolygon aPageFillPolygon(basegfx::tools::createPolygonFromRect(aPageFillRange));
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
                xRetval.realloc(1);
                const basegfx::BColor aRGBColor(aPageFillColor.getBColor());
                xRetval[0] = drawinglayer::primitive2d::Primitive2DReference(new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPageFillPolygon), aRGBColor));
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
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
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
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

            if(!rView.IsPageVisible() && rView.IsPageBorderVisible())
            {
                return false;
            }

            return true;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
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

            if(!rPage.GetLftBorder() && !rPage.GetUppBorder() && !rPage.GetRgtBorder() && !rPage.GetLwrBorder())
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
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContactOfPageHierarchy::ViewObjectContactOfPageHierarchy(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContactOfPageSubObject(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfPageHierarchy::~ViewObjectContactOfPageHierarchy()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfPageHierarchy::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            // process local sub-hierarchy
            const sal_uInt32 nSubHierarchyCount(GetViewContact().GetObjectCount());

            if(nSubHierarchyCount)
            {
                xRetval = getPrimitive2DSequenceSubHierarchy(rDisplayInfo);

                if(xRetval.hasElements())
                {
                    // get ranges
                    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
                    const basegfx::B2DRange aObjectRange(drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(xRetval, rViewInformation2D));
                    const basegfx::B2DRange aViewRange(rViewInformation2D.getViewport());

                    // check geometrical visibility
                    if(!aViewRange.isEmpty() && !aViewRange.overlaps(aObjectRange))
                    {
                        // not visible, release
                        xRetval.realloc(0);
                    }
                }
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
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

            if(static_cast< ViewContactOfGrid& >(GetViewContact()).getFront() != (bool)rView.IsGridFront())
            {
                return false;
            }

            return true;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfPageGrid::createPrimitive2DSequence(const DisplayInfo& /*rDisplayInfo*/) const
        {
            const SdrPageView* pPageView = GetObjectContact().TryToGetSdrPageView();
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            if(pPageView)
            {
                const SdrView& rView = pPageView->GetView();
                const SdrPage& rPage = getPage();
                const Color aGridColor(rView.GetGridColor());
                const basegfx::BColor aRGBGridColor(aGridColor.getBColor());

                basegfx::B2DHomMatrix aGridMatrix;
                aGridMatrix.set(0, 0, (double)(rPage.GetWdt() - (rPage.GetRgtBorder() + rPage.GetLftBorder())));
                aGridMatrix.set(1, 1, (double)(rPage.GetHgt() - (rPage.GetLwrBorder() + rPage.GetUppBorder())));
                aGridMatrix.set(0, 2, (double)rPage.GetLftBorder());
                aGridMatrix.set(1, 2, (double)rPage.GetUppBorder());

                const Size aRaw(rView.GetGridCoarse());
                const Size aFine(rView.GetGridFine());
                const double fWidthX(aRaw.getWidth());
                const double fWidthY(aRaw.getHeight());
                const sal_uInt32 nSubdivisionsX(aFine.getWidth() ? aRaw.getWidth() / aFine.getWidth() : 0L);
                const sal_uInt32 nSubdivisionsY(aFine.getHeight() ? aRaw.getHeight() / aFine.getHeight() : 0L);

                xRetval.realloc(1);
                xRetval[0] = drawinglayer::primitive2d::Primitive2DReference(new drawinglayer::primitive2d::GridPrimitive2D(
                    aGridMatrix, fWidthX, fWidthY, 10.0, 3.0, nSubdivisionsX, nSubdivisionsY, aRGBGridColor,
                    drawinglayer::primitive2d::createDefaultCross_3x3(aRGBGridColor)));
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
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

            if(static_cast< ViewContactOfHelplines& >(GetViewContact()).getFront() != (bool)rView.IsHlplFront())
            {
                return false;
            }

            return true;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfPageHelplines::createPrimitive2DSequence(const DisplayInfo& /*rDisplayInfo*/) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SdrPageView* pPageView = GetObjectContact().TryToGetSdrPageView();

            if(pPageView)
            {
                const SdrHelpLineList& rHelpLineList = pPageView->GetHelpLines();
                const sal_uInt32 nCount(rHelpLineList.GetCount());

                if(nCount)
                {
                    const basegfx::BColor aRGBColorA(1.0, 1.0, 1.0);
                    const basegfx::BColor aRGBColorB(0.0, 0.0, 0.0);
                    xRetval.realloc(nCount);

                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        const SdrHelpLine& rHelpLine = rHelpLineList[(sal_uInt16)a];
                        const basegfx::B2DPoint aPosition((double)rHelpLine.GetPos().X(), (double)rHelpLine.GetPos().Y());
                        const double fDiscreteDashLength(4.0);

                        switch(rHelpLine.GetKind())
                        {
                            default : // SDRHELPLINE_POINT
                            {
                                xRetval[a] = drawinglayer::primitive2d::Primitive2DReference(new drawinglayer::primitive2d::HelplinePrimitive2D(
                                    aPosition, basegfx::B2DVector(1.0, 0.0), drawinglayer::primitive2d::HELPLINESTYLE2D_POINT,
                                    aRGBColorA, aRGBColorB, fDiscreteDashLength));
                                break;
                            }
                            case SDRHELPLINE_VERTICAL :
                            {
                                xRetval[a] = drawinglayer::primitive2d::Primitive2DReference(new drawinglayer::primitive2d::HelplinePrimitive2D(
                                    aPosition, basegfx::B2DVector(0.0, 1.0), drawinglayer::primitive2d::HELPLINESTYLE2D_LINE,
                                    aRGBColorA, aRGBColorB, fDiscreteDashLength));
                                break;
                            }
                            case SDRHELPLINE_HORIZONTAL :
                            {
                                xRetval[a] = drawinglayer::primitive2d::Primitive2DReference(new drawinglayer::primitive2d::HelplinePrimitive2D(
                                    aPosition, basegfx::B2DVector(1.0, 0.0), drawinglayer::primitive2d::HELPLINESTYLE2D_LINE,
                                    aRGBColorA, aRGBColorB, fDiscreteDashLength));
                                break;
                            }
                        }
                    }
                }
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContactOfSdrPage::ViewObjectContactOfSdrPage(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContact(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfSdrPage::~ViewObjectContactOfSdrPage()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfSdrPage::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            // process local sub-hierarchy
            const sal_uInt32 nSubHierarchyCount(GetViewContact().GetObjectCount());

            if(nSubHierarchyCount)
            {
                const sal_Bool bDoGhostedDisplaying(
                    GetObjectContact().DoVisualizeEnteredGroup()
                    && !GetObjectContact().isOutputToPrinter()
                    && GetObjectContact().getActiveViewContact() == &GetViewContact());

                if(bDoGhostedDisplaying)
                {
                    rDisplayInfo.ClearGhostedDrawMode();
                }

                // create object hierarchy
                xRetval = getPrimitive2DSequenceSubHierarchy(rDisplayInfo);

                if(xRetval.hasElements())
                {
                    // get ranges
                    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
                    const basegfx::B2DRange aObjectRange(drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(xRetval, rViewInformation2D));
                    const basegfx::B2DRange aViewRange(rViewInformation2D.getViewport());

                    // check geometrical visibility
                    if(!aViewRange.isEmpty() && !aViewRange.overlaps(aObjectRange))
                    {
                        // not visible, release
                        xRetval.realloc(0);
                    }
                }

                if(bDoGhostedDisplaying)
                {
                    rDisplayInfo.SetGhostedDrawMode();
                }
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
