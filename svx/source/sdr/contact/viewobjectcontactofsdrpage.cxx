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

            if(!GetObjectContact().TryToGetSdrView())
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
            const SdrView* pSdrView = GetObjectContact().TryToGetSdrView();
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            if(pSdrView && pSdrView->GetSdrPageView())
            {
                Color aInitColor;

                if(pSdrView->IsPageVisible())
                {
                    aInitColor = pSdrView->GetSdrPageView()->GetApplicationBackgroundColor();
                }
                else
                {
                    aInitColor = pSdrView->GetSdrPageView()->GetApplicationDocumentColor();

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

            SdrView* pSdrView = GetObjectContact().TryToGetSdrView();

            if(!pSdrView || !pSdrView->GetSdrPageView())
            {
                return false;
            }

            if(!pSdrView->IsPageVisible())
            {
                return false;
            }

            return true;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfPageFill::createPrimitive2DSequence(const DisplayInfo& /*rDisplayInfo*/) const
        {
            const SdrView* pSdrView = GetObjectContact().TryToGetSdrView();
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            if(pSdrView && pSdrView->GetSdrPageView())
            {
                const SdrPage& rPage = getPage();
                const basegfx::B2DRange aPageFillRange(basegfx::B2DPoint(0.0, 0.0), rPage.GetPageScale());
                const basegfx::B2DPolygon aPageFillPolygon(basegfx::tools::createPolygonFromRect(aPageFillRange));
                Color aPageFillColor;

                if(pSdrView->GetSdrPageView()->GetApplicationDocumentColor() != COL_AUTO)
                {
                    aPageFillColor = pSdrView->GetSdrPageView()->GetApplicationDocumentColor();
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

            SdrView* pSdrView = GetObjectContact().TryToGetSdrView();

            if(!pSdrView)
            {
                return false;
            }

            if(!pSdrView->IsPageVisible())
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

            SdrView* pSdrView = GetObjectContact().TryToGetSdrView();

            if(!pSdrView)
            {
                return false;
            }

            if(!pSdrView->IsPageVisible() && pSdrView->IsPageBorderVisible())
            {
                return false;
            }

            if(!pSdrView->GetSdrPageView())
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

            SdrView* pSdrView = GetObjectContact().TryToGetSdrView();

            if(!pSdrView)
            {
                return false;
            }

            if(!pSdrView->IsBordVisible())
            {
                return false;
            }

            const SdrPage& rPage = getPage();

            if(!rPage.GetLeftPageBorder() && !rPage.GetTopPageBorder() && !rPage.GetRightPageBorder() && !rPage.GetBottomPageBorder())
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

            SdrView* pSdrView = GetObjectContact().TryToGetSdrView();

            if(!pSdrView)
            {
                return false;
            }

            if(!pSdrView->IsGridVisible())
            {
                return false;
            }

            // no page grid for preview renderers
            if(GetObjectContact().IsPreviewRenderer())
            {
                return false;
            }

            if(static_cast< ViewContactOfGrid& >(GetViewContact()).getFront() != pSdrView->IsGridFront())
            {
                return false;
            }

            return true;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfPageGrid::createPrimitive2DSequence(const DisplayInfo& /*rDisplayInfo*/) const
        {
            const SdrView* pSdrView = GetObjectContact().TryToGetSdrView();
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            if(pSdrView)
            {
                const SdrPage& rPage = getPage();
                const Color aGridColor(pSdrView->GetGridColor());
                const basegfx::BColor aRGBGridColor(aGridColor.getBColor());

                const basegfx::B2DRange aInnerRange(rPage.GetInnerPageRange());
                basegfx::B2DHomMatrix aGridMatrix;

                aGridMatrix.set(0, 0, aInnerRange.getWidth());
                aGridMatrix.set(1, 1, aInnerRange.getHeight());
                aGridMatrix.set(0, 2, aInnerRange.getMinX());
                aGridMatrix.set(1, 2, aInnerRange.getMinY());

                const Size aRaw(pSdrView->GetGridCoarse());
                const Size aFine(pSdrView->GetGridFine());
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

            SdrView* pSdrView = GetObjectContact().TryToGetSdrView();

            if(!pSdrView || !pSdrView->GetSdrPageView())
            {
                return false;
            }

            if(!pSdrView->IsHlplVisible())
            {
                return false;
            }

            // no helplines for preview renderers
            if(GetObjectContact().IsPreviewRenderer())
            {
                return false;
            }

            if(static_cast< ViewContactOfHelplines& >(GetViewContact()).getFront() != pSdrView->IsHlplFront())
            {
                return false;
            }

            return true;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfPageHelplines::createPrimitive2DSequence(const DisplayInfo& /*rDisplayInfo*/) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SdrView* pSdrView = GetObjectContact().TryToGetSdrView();

            if(pSdrView && pSdrView->GetSdrPageView())
            {
                const SdrHelpLineList& rHelpLineList = pSdrView->GetSdrPageView()->GetHelpLines();
                const sal_uInt32 nCount(rHelpLineList.GetCount());

                if(nCount)
                {
                    const basegfx::BColor aRGBColorA(1.0, 1.0, 1.0);
                    const basegfx::BColor aRGBColorB(0.0, 0.0, 0.0);
                    xRetval.realloc(nCount);

                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        const SdrHelpLine& rHelpLine = rHelpLineList[(sal_uInt16)a];
                        const double fDiscreteDashLength(4.0);

                        switch(rHelpLine.GetKind())
                        {
                            default : // SDRHELPLINE_POINT
                            {
                                xRetval[a] = drawinglayer::primitive2d::Primitive2DReference(
                                    new drawinglayer::primitive2d::HelplinePrimitive2D(
                                        rHelpLine.GetPos(),
                                        basegfx::B2DVector(1.0, 0.0),
                                        drawinglayer::primitive2d::HELPLINESTYLE2D_POINT,
                                        aRGBColorA,
                                        aRGBColorB,
                                        fDiscreteDashLength));
                                break;
                            }
                            case SDRHELPLINE_VERTICAL :
                            {
                                xRetval[a] = drawinglayer::primitive2d::Primitive2DReference(
                                    new drawinglayer::primitive2d::HelplinePrimitive2D(
                                        rHelpLine.GetPos(),
                                        basegfx::B2DVector(0.0, 1.0),
                                        drawinglayer::primitive2d::HELPLINESTYLE2D_LINE,
                                        aRGBColorA,
                                        aRGBColorB,
                                        fDiscreteDashLength));
                                break;
                            }
                            case SDRHELPLINE_HORIZONTAL :
                            {
                                xRetval[a] = drawinglayer::primitive2d::Primitive2DReference(
                                    new drawinglayer::primitive2d::HelplinePrimitive2D(
                                        rHelpLine.GetPos(),
                                        basegfx::B2DVector(1.0, 0.0),
                                        drawinglayer::primitive2d::HELPLINESTYLE2D_LINE,
                                        aRGBColorA,
                                        aRGBColorB,
                                        fDiscreteDashLength));
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
