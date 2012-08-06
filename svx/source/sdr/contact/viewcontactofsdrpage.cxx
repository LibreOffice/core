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

#include <svx/sdr/contact/viewcontactofsdrpage.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrpage.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <vcl/lazydelete.hxx>
#include <svx/svdstr.hrc>
#include <svx/svdglob.hxx>
#include <drawinglayer/primitive2d/discreteshadowprimitive2d.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfPageSubObject::ViewContactOfPageSubObject(ViewContactOfSdrPage& rParentViewContactOfSdrPage)
        :   mrParentViewContactOfSdrPage(rParentViewContactOfSdrPage)
        {
        }

        ViewContactOfPageSubObject::~ViewContactOfPageSubObject()
        {
        }

        ViewContact* ViewContactOfPageSubObject::GetParentContact() const
        {
            return &mrParentViewContactOfSdrPage;
        }

        const SdrPage& ViewContactOfPageSubObject::getPage() const
        {
            return mrParentViewContactOfSdrPage.GetSdrPage();
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfPageBackground::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfPageBackground(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfPageBackground::createViewIndependentPrimitive2DSequence() const
        {
            // We have only the page information, not the view information. Use the
            // svtools::DOCCOLOR color for initialisation
            const svtools::ColorConfig aColorConfig;
            const Color aInitColor(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);
            const basegfx::BColor aRGBColor(aInitColor.getBColor());
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::BackgroundColorPrimitive2D(aRGBColor));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }

        ViewContactOfPageBackground::ViewContactOfPageBackground(ViewContactOfSdrPage& rParentViewContactOfSdrPage)
        :   ViewContactOfPageSubObject(rParentViewContactOfSdrPage)
        {
        }

        ViewContactOfPageBackground::~ViewContactOfPageBackground()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfPageShadow::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfPageShadow(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfPageShadow::createViewIndependentPrimitive2DSequence() const
        {
            static bool bUseOldPageShadow(false);
            const SdrPage& rPage = getPage();
            basegfx::B2DHomMatrix aPageMatrix;
            aPageMatrix.set(0, 0, (double)rPage.GetWdt());
            aPageMatrix.set(1, 1, (double)rPage.GetHgt());

            if(bUseOldPageShadow)
            {
                // create page shadow polygon
                const double fPageBorderFactor(1.0 / 256.0);
                basegfx::B2DPolygon aPageShadowPolygon;
                aPageShadowPolygon.append(basegfx::B2DPoint(1.0, fPageBorderFactor));
                aPageShadowPolygon.append(basegfx::B2DPoint(1.0 + fPageBorderFactor, fPageBorderFactor));
                aPageShadowPolygon.append(basegfx::B2DPoint(1.0 + fPageBorderFactor, 1.0 + fPageBorderFactor));
                aPageShadowPolygon.append(basegfx::B2DPoint(fPageBorderFactor, 1.0 + fPageBorderFactor));
                aPageShadowPolygon.append(basegfx::B2DPoint(fPageBorderFactor, 1.0));
                aPageShadowPolygon.append(basegfx::B2DPoint(1.0, 1.0));
                aPageShadowPolygon.setClosed(true);
                aPageShadowPolygon.transform(aPageMatrix);

                // We have only the page information, not the view information. Use the
                // svtools::FONTCOLOR color for initialisation
                const svtools::ColorConfig aColorConfig;
                const Color aShadowColor(aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor);
                const basegfx::BColor aRGBShadowColor(aShadowColor.getBColor());
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(aPageShadowPolygon),
                        aRGBShadowColor));

                return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }
            else
            {
                static vcl::DeleteOnDeinit<drawinglayer::primitive2d::DiscreteShadow>
                    aDiscreteShadow(new drawinglayer::primitive2d::DiscreteShadow(
                        BitmapEx(ResId(SIP_SA_PAGESHADOW35X35, *ImpGetResMgr()))));
                if (aDiscreteShadow.get() != NULL)
                {
                    const drawinglayer::primitive2d::Primitive2DReference xReference(
                        new drawinglayer::primitive2d::DiscreteShadowPrimitive2D(
                            aPageMatrix,
                                *aDiscreteShadow.get()));

                    return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                }
                return drawinglayer::primitive2d::Primitive2DSequence();
            }
        }

        ViewContactOfPageShadow::ViewContactOfPageShadow(ViewContactOfSdrPage& rParentViewContactOfSdrPage)
        :   ViewContactOfPageSubObject(rParentViewContactOfSdrPage)
        {
        }

        ViewContactOfPageShadow::~ViewContactOfPageShadow()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfMasterPage::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfMasterPage(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfMasterPage::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            // this class is used when the page is a MasterPage and is responsible to
            // create a visualisation for the MPBGO, if exists. This needs to be suppressed
            // when a SdrPage which uses a MasterPage creates it's output. Suppression
            // is done in the corresponding VOC since DisplayInfo data is needed
            const SdrPage& rPage = getPage();

            if(rPage.IsMasterPage())
            {
                if(0 == rPage.GetPageNum())
                {
                    // #i98063#
                    // filter MasterPage 0 since it's the HandoutPage. Thus, it's a
                    // MasterPage, but has no MPBGO, so there is nothing to do here.
                }
                else
                {
                    drawinglayer::attribute::SdrFillAttribute aFill;

                    // #i110846# Suppress SdrPage FillStyle for MasterPages without StyleSheets,
                    // else the PoolDefault (XFILL_COLOR and Blue8) will be used. Normally, all
                    // MasterPages should have a StyleSheet excactly for this reason, but historically
                    // e.g. the Notes MasterPage has no StyleSheet set (and there maybe others).
                    if(rPage.getSdrPageProperties().GetStyleSheet())
                    {
                        // create page fill attributes with correct properties
                        aFill = drawinglayer::primitive2d::createNewSdrFillAttribute(
                            rPage.getSdrPageProperties().GetItemSet());
                    }

                    if(!aFill.isDefault())
                    {
                        // direct model data is the page size, get and use it
                        const basegfx::B2DRange aInnerRange(
                            rPage.GetLftBorder(), rPage.GetUppBorder(),
                            rPage.GetWdt() - rPage.GetRgtBorder(), rPage.GetHgt() - rPage.GetLwrBorder());
                        const basegfx::B2DPolygon aInnerPolgon(basegfx::tools::createPolygonFromRect(aInnerRange));
                        const basegfx::B2DHomMatrix aEmptyTransform;
                        const drawinglayer::primitive2d::Primitive2DReference xReference(
                            drawinglayer::primitive2d::createPolyPolygonFillPrimitive(
                                basegfx::B2DPolyPolygon(aInnerPolgon),
                                aEmptyTransform,
                                aFill,
                                drawinglayer::attribute::FillGradientAttribute()));

                        xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                    }
                }
            }

            return xRetval;
        }

        ViewContactOfMasterPage::ViewContactOfMasterPage(ViewContactOfSdrPage& rParentViewContactOfSdrPage)
        :   ViewContactOfPageSubObject(rParentViewContactOfSdrPage)
        {
        }

        ViewContactOfMasterPage::~ViewContactOfMasterPage()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfPageFill::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfPageFill(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfPageFill::createViewIndependentPrimitive2DSequence() const
        {
            const SdrPage& rPage = getPage();
            const basegfx::B2DRange aPageFillRange(0.0, 0.0, (double)rPage.GetWdt(), (double)rPage.GetHgt());
            const basegfx::B2DPolygon aPageFillPolygon(basegfx::tools::createPolygonFromRect(aPageFillRange));

            // We have only the page information, not the view information. Use the
            // svtools::DOCCOLOR color for initialisation
            const svtools::ColorConfig aColorConfig;
            const Color aPageFillColor(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);

            // create and add primitive
            const basegfx::BColor aRGBColor(aPageFillColor.getBColor());
            const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPageFillPolygon), aRGBColor));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }

        ViewContactOfPageFill::ViewContactOfPageFill(ViewContactOfSdrPage& rParentViewContactOfSdrPage)
        :   ViewContactOfPageSubObject(rParentViewContactOfSdrPage)
        {
        }

        ViewContactOfPageFill::~ViewContactOfPageFill()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfOuterPageBorder::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfOuterPageBorder(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfOuterPageBorder::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SdrPage& rPage = getPage();
            const basegfx::B2DRange aPageBorderRange(0.0, 0.0, (double)rPage.GetWdt(), (double)rPage.GetHgt());

            // Changed to 0x949599 for renaissance, before svtools::FONTCOLOR was used.
            // Added old case as fallback for HighContrast.
            basegfx::BColor aRGBBorderColor(0x94 / (double)0xff, 0x95 / (double)0xff, 0x99 / (double)0xff);

            if(Application::GetSettings().GetStyleSettings().GetHighContrastMode())
            {
                const svtools::ColorConfig aColorConfig;
                const Color aBorderColor(aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor);

                aRGBBorderColor = aBorderColor.getBColor();
            }

            if(rPage.getPageBorderOnlyLeftRight())
            {
                // #i93597# for Report Designer, the page border shall be only displayed right and left,
                // but not top and bottom. Create simplified geometry.
                basegfx::B2DPolygon aLeft, aRight;

                aLeft.append(basegfx::B2DPoint(aPageBorderRange.getMinX(), aPageBorderRange.getMinY()));
                aLeft.append(basegfx::B2DPoint(aPageBorderRange.getMinX(), aPageBorderRange.getMaxY()));

                aRight.append(basegfx::B2DPoint(aPageBorderRange.getMaxX(), aPageBorderRange.getMinY()));
                aRight.append(basegfx::B2DPoint(aPageBorderRange.getMaxX(), aPageBorderRange.getMaxY()));

                xRetval.realloc(2);
                xRetval[0] = drawinglayer::primitive2d::Primitive2DReference(new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aLeft, aRGBBorderColor));
                xRetval[1] = drawinglayer::primitive2d::Primitive2DReference(new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aRight, aRGBBorderColor));
            }
            else
            {
                xRetval.realloc(1);
                const basegfx::B2DPolygon aPageBorderPolygon(basegfx::tools::createPolygonFromRect(aPageBorderRange));
                xRetval[0] = drawinglayer::primitive2d::Primitive2DReference(new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aPageBorderPolygon, aRGBBorderColor));
            }

            return xRetval;
        }

        ViewContactOfOuterPageBorder::ViewContactOfOuterPageBorder(ViewContactOfSdrPage& rParentViewContactOfSdrPage)
        :   ViewContactOfPageSubObject(rParentViewContactOfSdrPage)
        {
        }

        ViewContactOfOuterPageBorder::~ViewContactOfOuterPageBorder()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfInnerPageBorder::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfInnerPageBorder(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfInnerPageBorder::createViewIndependentPrimitive2DSequence() const
        {
            const SdrPage& rPage = getPage();
            const basegfx::B2DRange aPageBorderRange(
                (double)rPage.GetLftBorder(), (double)rPage.GetUppBorder(),
                (double)(rPage.GetWdt() - rPage.GetRgtBorder()), (double)(rPage.GetHgt() - rPage.GetLwrBorder()));
            const basegfx::B2DPolygon aPageBorderPolygon(basegfx::tools::createPolygonFromRect(aPageBorderRange));

            // We have only the page information, not the view information. Use the
            // svtools::FONTCOLOR or svtools::DOCBOUNDARIES color for initialisation
            const svtools::ColorConfig aColorConfig;
            Color aBorderColor;

            if(Application::GetSettings().GetStyleSettings().GetHighContrastMode())
            {
                aBorderColor = aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor;
            }
            else
            {
                aBorderColor = aColorConfig.GetColorValue(svtools::DOCBOUNDARIES).nColor;
            }

            // create page outer border primitive
            const basegfx::BColor aRGBBorderColor(aBorderColor.getBColor());
            const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aPageBorderPolygon, aRGBBorderColor));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }

        ViewContactOfInnerPageBorder::ViewContactOfInnerPageBorder(ViewContactOfSdrPage& rParentViewContactOfSdrPage)
        :   ViewContactOfPageSubObject(rParentViewContactOfSdrPage)
        {
        }

        ViewContactOfInnerPageBorder::~ViewContactOfInnerPageBorder()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfPageHierarchy::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfPageHierarchy(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfPageHierarchy::createViewIndependentPrimitive2DSequence() const
        {
            // collect sub-hierarchy
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const sal_uInt32 nObjectCount(GetObjectCount());

            // collect all sub-primitives
            for(sal_uInt32 a(0); a < nObjectCount; a++)
            {
                const ViewContact& rCandidate(GetViewContact(a));
                const drawinglayer::primitive2d::Primitive2DSequence aCandSeq(rCandidate.getViewIndependentPrimitive2DSequence());

                drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, aCandSeq);
            }

            return xRetval;
        }

        ViewContactOfPageHierarchy::ViewContactOfPageHierarchy(ViewContactOfSdrPage& rParentViewContactOfSdrPage)
        :   ViewContactOfPageSubObject(rParentViewContactOfSdrPage)
        {
        }

        ViewContactOfPageHierarchy::~ViewContactOfPageHierarchy()
        {
        }

        sal_uInt32 ViewContactOfPageHierarchy::GetObjectCount() const
        {
            return getPage().GetObjCount();
        }

        ViewContact& ViewContactOfPageHierarchy::GetViewContact(sal_uInt32 nIndex) const
        {
            SdrObject* pObj = getPage().GetObj(nIndex);
            DBG_ASSERT(pObj, "ViewContactOfPageHierarchy::GetViewContact: Corrupt SdrObjList (!)");
            return pObj->GetViewContact();
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfGrid::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfPageGrid(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfGrid::createViewIndependentPrimitive2DSequence() const
        {
            // We have only the page information, not the view information and thus no grid settings. Create empty
            // default. For the view-dependent implementation, see ViewObjectContactOfPageGrid::createPrimitive2DSequence
            return drawinglayer::primitive2d::Primitive2DSequence();
        }

        ViewContactOfGrid::ViewContactOfGrid(ViewContactOfSdrPage& rParentViewContactOfSdrPage, bool bFront)
        :   ViewContactOfPageSubObject(rParentViewContactOfSdrPage),
            mbFront(bFront)
        {
        }

        ViewContactOfGrid::~ViewContactOfGrid()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfHelplines::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfPageHelplines(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfHelplines::createViewIndependentPrimitive2DSequence() const
        {
            // We have only the page information, not the view information and thus no helplines. Create empty
            // default. For the view-dependent implementation, see ViewObjectContactOfPageHelplines::createPrimitive2DSequence
            return drawinglayer::primitive2d::Primitive2DSequence();
        }

        ViewContactOfHelplines::ViewContactOfHelplines(ViewContactOfSdrPage& rParentViewContactOfSdrPage, bool bFront)
        :   ViewContactOfPageSubObject(rParentViewContactOfSdrPage),
            mbFront(bFront)
        {
        }

        ViewContactOfHelplines::~ViewContactOfHelplines()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something.
        ViewObjectContact& ViewContactOfSdrPage::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfSdrPage(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        ViewContactOfSdrPage::ViewContactOfSdrPage(SdrPage& rPage)
        :   ViewContact(),
            mrPage(rPage),
            maViewContactOfPageBackground(*this),
            maViewContactOfPageShadow(*this),
            maViewContactOfPageFill(*this),
            maViewContactOfMasterPage(*this),
            maViewContactOfOuterPageBorder(*this),
            maViewContactOfInnerPageBorder(*this),
            maViewContactOfGridBack(*this, false),
            maViewContactOfHelplinesBack(*this, false),
            maViewContactOfPageHierarchy(*this),
            maViewContactOfGridFront(*this, true),
            maViewContactOfHelplinesFront(*this, true)
        {
        }

        ViewContactOfSdrPage::~ViewContactOfSdrPage()
        {
        }

        // Access to possible sub-hierarchy
        sal_uInt32 ViewContactOfSdrPage::GetObjectCount() const
        {
            // Fixed count of content. It contains PageBackground (Wiese), PageShadow, PageFill,
            // then - depending on if the page has a MasterPage - either MasterPage Hierarchy
            // or MPBGO. Also OuterPageBorder, InnerPageBorder and two pairs of Grid and Helplines
            // (for front and back) which internally are visible or not depending on the current
            // front/back setting for those.
            return 11;
        }

        ViewContact& ViewContactOfSdrPage::GetViewContact(sal_uInt32 nIndex) const
        {
            switch(nIndex)
            {
                case 0: return (ViewContact&)maViewContactOfPageBackground;
                case 1: return (ViewContact&)maViewContactOfPageShadow;
                case 2: return (ViewContact&)maViewContactOfPageFill;
                case 3:
                {
                    const SdrPage& rPage = GetSdrPage();

                    if(rPage.TRG_HasMasterPage())
                    {
                        return rPage.TRG_GetMasterPageDescriptorViewContact();
                    }
                    else
                    {
                        return (ViewContact&)maViewContactOfMasterPage;
                    }
                }
                case 4: return (ViewContact&)maViewContactOfOuterPageBorder;
                case 5: return (ViewContact&)maViewContactOfInnerPageBorder;
                case 6: return (ViewContact&)maViewContactOfGridBack;
                case 7: return (ViewContact&)maViewContactOfHelplinesBack;
                case 8: return (ViewContact&)maViewContactOfPageHierarchy;
                case 9: return (ViewContact&)maViewContactOfGridFront;
                default: return (ViewContact&)maViewContactOfHelplinesFront;
            }
        }

        // React on changes of the object of this ViewContact
        void ViewContactOfSdrPage::ActionChanged()
        {
            // call parent
            ViewContact::ActionChanged();

            // apply to local viewContacts, they all rely on page information. Exception
            // is the sub hierarchy; this will not be influenced by the change
            maViewContactOfPageBackground.ActionChanged();
            maViewContactOfPageShadow.ActionChanged();
            maViewContactOfPageFill.ActionChanged();

            const SdrPage& rPage = GetSdrPage();

            if(rPage.TRG_HasMasterPage())
            {
                rPage.TRG_GetMasterPageDescriptorViewContact().ActionChanged();
            }
            else if(rPage.IsMasterPage())
            {
                maViewContactOfMasterPage.ActionChanged();
            }

            maViewContactOfOuterPageBorder.ActionChanged();
            maViewContactOfInnerPageBorder.ActionChanged();
            maViewContactOfGridBack.ActionChanged();
            maViewContactOfHelplinesBack.ActionChanged();
            maViewContactOfGridFront.ActionChanged();
            maViewContactOfHelplinesFront.ActionChanged();
        }

        // overload for acessing the SdrPage
        SdrPage* ViewContactOfSdrPage::TryToGetSdrPage() const
        {
            return &GetSdrPage();
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrPage::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            // collect all sub-sequences including sub hierarchy.
            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, maViewContactOfPageBackground.getViewIndependentPrimitive2DSequence());
            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, maViewContactOfPageShadow.getViewIndependentPrimitive2DSequence());
            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, maViewContactOfPageFill.getViewIndependentPrimitive2DSequence());

            const SdrPage& rPage = GetSdrPage();

            if(rPage.TRG_HasMasterPage())
            {
                drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval,
                    rPage.TRG_GetMasterPageDescriptorViewContact().getViewIndependentPrimitive2DSequence());
            }
            else if(rPage.IsMasterPage())
            {
                drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval,
                    maViewContactOfMasterPage.getViewIndependentPrimitive2DSequence());
            }

            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, maViewContactOfOuterPageBorder.getViewIndependentPrimitive2DSequence());
            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, maViewContactOfInnerPageBorder.getViewIndependentPrimitive2DSequence());
            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, maViewContactOfPageHierarchy.getViewIndependentPrimitive2DSequence());

            // Only add front versions of grid and helplines since no visibility test is done,
            // so adding the back incarnations is not necessary. This makes the Front
            // visualisation the default when no visibility tests are done.
            //
            // Since we have no view here, no grid and helpline definitions are available currently. The used
            // methods at ViewContactOfHelplines and ViewContactOfGrid return only empty sequences and
            // do not need to be called ATM. This may change later if grid or helpline info gets
            // model data (it should not). Keeping the lines commented to hold this hint.
            //
            // drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, maViewContactOfGridFront.getViewIndependentPrimitive2DSequence());
            // drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, maViewContactOfHelplinesFront.getViewIndependentPrimitive2DSequence());

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
