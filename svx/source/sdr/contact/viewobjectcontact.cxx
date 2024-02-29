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

#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/animation/animationstate.hxx>
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/animatedprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/structuretagprimitive2d.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdotext.hxx>
#include <vcl/pdfwriter.hxx>
#include <vcl/pdfextoutdevdata.hxx>

using namespace com::sun::star;

namespace {

// animated extractor

// Necessary to filter a sequence of animated primitives from
// a sequence of primitives to find out if animated or not. The decision for
// what to decompose is hard-coded and only done for knowingly animated primitives
// to not decompose too deeply and unnecessarily. This implies that the list
// which is view-specific needs to be expanded by hand when new animated objects
// are added. This may eventually be changed to a dynamically configurable approach
// if necessary.
class AnimatedExtractingProcessor2D : public drawinglayer::processor2d::BaseProcessor2D
{
protected:
    // the found animated primitives
    drawinglayer::primitive2d::Primitive2DContainer  maPrimitive2DSequence;

    // text animation allowed?
    bool                                            mbTextAnimationAllowed : 1;

    // graphic animation allowed?
    bool                                            mbGraphicAnimationAllowed : 1;

    // as tooling, the process() implementation takes over API handling and calls this
    // virtual render method when the primitive implementation is BasePrimitive2D-based.
    virtual void processBasePrimitive2D(const drawinglayer::primitive2d::BasePrimitive2D& rCandidate) override;

public:
    AnimatedExtractingProcessor2D(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation,
        bool bTextAnimationAllowed,
        bool bGraphicAnimationAllowed);

    // data access
    const drawinglayer::primitive2d::Primitive2DContainer& getPrimitive2DSequence() const { return maPrimitive2DSequence; }
    drawinglayer::primitive2d::Primitive2DContainer extractPrimitive2DSequence() { return std::move(maPrimitive2DSequence); }
};

AnimatedExtractingProcessor2D::AnimatedExtractingProcessor2D(
    const drawinglayer::geometry::ViewInformation2D& rViewInformation,
    bool bTextAnimationAllowed,
    bool bGraphicAnimationAllowed)
:   drawinglayer::processor2d::BaseProcessor2D(rViewInformation),
    mbTextAnimationAllowed(bTextAnimationAllowed),
    mbGraphicAnimationAllowed(bGraphicAnimationAllowed)
{
}

void AnimatedExtractingProcessor2D::processBasePrimitive2D(const drawinglayer::primitive2d::BasePrimitive2D& rCandidate)
{
    // known implementation, access directly
    switch(rCandidate.getPrimitive2DID())
    {
        // add and accept animated primitives directly, no need to decompose
        case PRIMITIVE2D_ID_ANIMATEDSWITCHPRIMITIVE2D :
        case PRIMITIVE2D_ID_ANIMATEDBLINKPRIMITIVE2D :
        case PRIMITIVE2D_ID_ANIMATEDINTERPOLATEPRIMITIVE2D :
        {
            const drawinglayer::primitive2d::AnimatedSwitchPrimitive2D& rSwitchPrimitive = static_cast< const drawinglayer::primitive2d::AnimatedSwitchPrimitive2D& >(rCandidate);

            if((rSwitchPrimitive.isTextAnimation() && mbTextAnimationAllowed)
                || (rSwitchPrimitive.isGraphicAnimation() && mbGraphicAnimationAllowed))
            {
                const drawinglayer::primitive2d::Primitive2DReference xReference(const_cast< drawinglayer::primitive2d::BasePrimitive2D* >(&rCandidate));
                maPrimitive2DSequence.push_back(xReference);
            }
            break;
        }

        // decompose animated gifs where SdrGrafPrimitive2D produces a GraphicPrimitive2D
        // which then produces the animation infos (all when used/needed)
        case PRIMITIVE2D_ID_SDRGRAFPRIMITIVE2D :
        case PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D :

        // decompose SdrObjects with evtl. animated text
        case PRIMITIVE2D_ID_SDRCAPTIONPRIMITIVE2D :
        case PRIMITIVE2D_ID_SDRCONNECTORPRIMITIVE2D :
        case PRIMITIVE2D_ID_SDRCUSTOMSHAPEPRIMITIVE2D :
        case PRIMITIVE2D_ID_SDRELLIPSEPRIMITIVE2D :
        case PRIMITIVE2D_ID_SDRELLIPSESEGMENTPRIMITIVE2D :
        case PRIMITIVE2D_ID_SDRMEASUREPRIMITIVE2D :
        case PRIMITIVE2D_ID_SDRPATHPRIMITIVE2D :
        case PRIMITIVE2D_ID_SDRRECTANGLEPRIMITIVE2D :

        // #121194# With Graphic as Bitmap FillStyle, also check
        // for primitives filled with animated graphics
        case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D:
        case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D:
        case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:

        // decompose evtl. animated text contained in MaskPrimitive2D
        // or group primitives
        case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
        case PRIMITIVE2D_ID_GROUPPRIMITIVE2D :
        {
            process(rCandidate);
            break;
        }

        default :
        {
            // nothing to do for the rest
            break;
        }
    }
}

} // end of anonymous namespace

namespace sdr::contact {

ViewObjectContact::ViewObjectContact(ObjectContact& rObjectContact, ViewContact& rViewContact)
:   mrObjectContact(rObjectContact),
    mrViewContact(rViewContact),
    maGridOffset(0.0, 0.0),
    mnActionChangedCount(0),
    mbLazyInvalidate(false)
{
    // make the ViewContact remember me
    mrViewContact.AddViewObjectContact(*this);

    // make the ObjectContact remember me
    mrObjectContact.AddViewObjectContact(*this);
}

ViewObjectContact::~ViewObjectContact()
{
    // if the object range is empty, then we have never had the primitive range change, so nothing to invalidate
    if (!maObjectRange.isEmpty())
    {
        // invalidate in view
        if(!getObjectRange().isEmpty())
        {
            GetObjectContact().InvalidatePartOfView(maObjectRange);
        }
    }

    // delete PrimitiveAnimation
    mpPrimitiveAnimation.reset();

    // take care of remembered ObjectContact. Remove from
    // OC first. The VC removal (below) CAN trigger a StopGettingViewed()
    // which (depending of its implementation) may destroy other OCs. This
    // can trigger the deletion of the helper OC of a page visualising object
    // which IS the OC of this object. Eventually StopGettingViewed() needs
    // to get asynchron later
    GetObjectContact().RemoveViewObjectContact(*this);

    // take care of remembered ViewContact
    GetViewContact().RemoveViewObjectContact(*this);
}

const basegfx::B2DRange& ViewObjectContact::getObjectRange() const
{
    if(maObjectRange.isEmpty())
    {
        const drawinglayer::geometry::ViewInformation2D& rViewInfo2D = GetObjectContact().getViewInformation2D();
        basegfx::B2DRange aTempRange = GetViewContact().getRange(rViewInfo2D);
        if (!aTempRange.isEmpty())
        {
            const_cast< ViewObjectContact* >(this)->maObjectRange = aTempRange;
        }
        else
        {
            // if range is not computed (new or LazyInvalidate objects), force it
            const DisplayInfo aDisplayInfo;
            const drawinglayer::primitive2d::Primitive2DContainer& xSequence(getPrimitive2DSequence(aDisplayInfo));

            if(!xSequence.empty())
            {
                const_cast< ViewObjectContact* >(this)->maObjectRange =
                    xSequence.getB2DRange(rViewInfo2D);
            }
        }
    }

    return maObjectRange;
}

void ViewObjectContact::ActionChanged()
{
    // clear cached primitives
    mxPrimitive2DSequence.clear();
    ++mnActionChangedCount;

    if(mbLazyInvalidate)
        return;

    // set local flag
    mbLazyInvalidate = true;

    // force ObjectRange
    getObjectRange();

    if(!getObjectRange().isEmpty())
    {
        // invalidate current valid range
        GetObjectContact().InvalidatePartOfView(maObjectRange);

        // reset gridOffset, it needs to be recalculated
        if (GetObjectContact().supportsGridOffsets())
            resetGridOffset();
        else
            maObjectRange.reset();
    }

    // register at OC for lazy invalidate
    GetObjectContact().setLazyInvalidate(*this);
}

// IASS: helper for IASS invalidates
void ViewObjectContact::ActionChangedIfDifferentPageView(SdrPageView& rSdrPageView)
{
    SdrPageView* pSdrPageView(GetObjectContact().TryToGetSdrPageView());

    // if there is no SdrPageView or different from given one, force
    // invalidate/repaint
    if (nullptr == pSdrPageView || pSdrPageView != &rSdrPageView)
        ActionChanged();
}

void ViewObjectContact::triggerLazyInvalidate()
{
    if(!mbLazyInvalidate)
        return;

    // reset flag
    mbLazyInvalidate = false;

    // force ObjectRange
    getObjectRange();

    if(!getObjectRange().isEmpty())
    {
        // invalidate current valid range
        GetObjectContact().InvalidatePartOfView(maObjectRange);
    }
}

// Take some action when new objects are inserted
void ViewObjectContact::ActionChildInserted(ViewContact& rChild)
{
    // force creation of the new VOC and trigger it's refresh, so it
    // will take part in LazyInvalidate immediately
    rChild.GetViewObjectContact(GetObjectContact()).ActionChanged();

    // forward action to ObjectContact
    // const ViewObjectContact& rChildVOC = rChild.GetViewObjectContact(GetObjectContact());
    // GetObjectContact().InvalidatePartOfView(rChildVOC.getObjectRange());
}

void ViewObjectContact::checkForPrimitive2DAnimations()
{
    // remove old one
    mpPrimitiveAnimation.reset();

    // check for animated primitives
    if(mxPrimitive2DSequence.empty())
        return;

    const bool bTextAnimationAllowed(GetObjectContact().IsTextAnimationAllowed());
    const bool bGraphicAnimationAllowed(GetObjectContact().IsGraphicAnimationAllowed());

    if(bTextAnimationAllowed || bGraphicAnimationAllowed)
    {
        AnimatedExtractingProcessor2D aAnimatedExtractor(GetObjectContact().getViewInformation2D(),
            bTextAnimationAllowed, bGraphicAnimationAllowed);
        aAnimatedExtractor.process(mxPrimitive2DSequence);

        if(!aAnimatedExtractor.getPrimitive2DSequence().empty())
        {
            // derived primitiveList is animated, setup new PrimitiveAnimation
            mpPrimitiveAnimation.reset( new sdr::animation::PrimitiveAnimation(*this, aAnimatedExtractor.extractPrimitive2DSequence()) );
        }
    }
}

void ViewObjectContact::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
{
    // get the view-independent Primitive from the viewContact
    drawinglayer::primitive2d::Primitive2DContainer xRetval;
    GetViewContact().getViewIndependentPrimitive2DContainer(xRetval);

    if(!xRetval.empty())
    {
        // handle GluePoint
        if(!GetObjectContact().isOutputToPrinter() && GetObjectContact().AreGluePointsVisible())
        {
            const drawinglayer::primitive2d::Primitive2DContainer xGlue(GetViewContact().createGluePointPrimitive2DSequence());

            if(!xGlue.empty())
            {
                xRetval.append(xGlue);
            }
        }

        // handle ghosted
        if(isPrimitiveGhosted(rDisplayInfo))
        {
            const basegfx::BColor aRGBWhite(1.0, 1.0, 1.0);
            const basegfx::BColorModifierSharedPtr aBColorModifier =
                std::make_shared<basegfx::BColorModifier_interpolate>(
                    aRGBWhite,
                    0.5);
            drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                    std::move(xRetval),
                    aBColorModifier));

            xRetval = drawinglayer::primitive2d::Primitive2DContainer { xReference };
        }
    }

    rVisitor.visit(xRetval);
}

bool ViewObjectContact::isExportPDFTags() const
{
    return GetObjectContact().isExportTaggedPDF();
}

/** Check if we need to embed to a StructureTagPrimitive2D, too. This
    was done at ImplRenderPaintProc::createRedirectedPrimitive2DSequence before
*/
void ViewObjectContact::createStructureTag(drawinglayer::primitive2d::Primitive2DContainer & rNewPrimitiveSequence) const
{
    SdrObject *const pSdrObj(mrViewContact.TryToGetSdrObject());

    // Check if we need to embed to a StructureTagPrimitive2D, too. This
    // was done at ImplRenderPaintProc::createRedirectedPrimitive2DSequence before
    if (!rNewPrimitiveSequence.empty() && isExportPDFTags()
        // ISO 14289-1:2014, Clause: 7.3
        && (!pSdrObj || pSdrObj->getParentSdrObjectFromSdrObject() == nullptr))
    {
        if (nullptr != pSdrObj && !pSdrObj->IsDecorative())
        {
            vcl::PDFWriter::StructElement eElement(vcl::PDFWriter::NonStructElement);
            const SdrInventor nInventor(pSdrObj->GetObjInventor());
            const SdrObjKind nIdentifier(pSdrObj->GetObjIdentifier());
            const bool bIsTextObj(nullptr != DynCastSdrTextObj(pSdrObj));

            // Note: SwFlyDrawObj/SwVirtFlyDrawObj have SdrInventor::Swg - these
            // are *not* handled here because not all of them are painted
            // completely with primitives, so a tag here does not encapsulate them.
            // The tag must be created by SwTaggedPDFHelper until this is fixed.
            if ( nInventor == SdrInventor::Default )
            {
                if ( nIdentifier == SdrObjKind::Group )
                    eElement = vcl::PDFWriter::Figure;
                else if (nIdentifier == SdrObjKind::Table)
                    eElement = vcl::PDFWriter::Table;
                else if (nIdentifier == SdrObjKind::Media)
                    eElement = vcl::PDFWriter::Annot;
                else if ( nIdentifier == SdrObjKind::TitleText )
                    eElement = vcl::PDFWriter::Heading;
                else if ( nIdentifier == SdrObjKind::OutlineText )
                    eElement = vcl::PDFWriter::Division;
                else if ( !bIsTextObj || !static_cast<const SdrTextObj&>(*pSdrObj).HasText() )
                    eElement = vcl::PDFWriter::Figure;
                else
                    eElement = vcl::PDFWriter::Division;
            }

            if(vcl::PDFWriter::NonStructElement != eElement)
            {
                SdrPage* pSdrPage(pSdrObj->getSdrPageFromSdrObject());

                if(pSdrPage)
                {
                    const bool bBackground(pSdrPage->IsMasterPage());
                    const bool bImage(SdrObjKind::Graphic == pSdrObj->GetObjIdentifier());
                    // note: there must be output device here, in PDF export
                    void const* pAnchorKey(nullptr);
                    if (auto const pUserCall = pSdrObj->GetUserCall())
                    {
                        pAnchorKey = pUserCall->GetPDFAnchorStructureElementKey(*pSdrObj);
                    }

                    ::std::vector<sal_Int32> annotIds;
                    if (eElement == vcl::PDFWriter::Annot
                        && !static_cast<SdrMediaObj*>(pSdrObj)->getURL().isEmpty())
                    {
                        auto const pPDFExtOutDevData(GetObjectContact().GetPDFExtOutDevData());
                        assert(pPDFExtOutDevData);
                        annotIds = pPDFExtOutDevData->GetScreenAnnotIds(pSdrObj);
                    }

                    drawinglayer::primitive2d::Primitive2DReference xReference(
                        new drawinglayer::primitive2d::StructureTagPrimitive2D(
                            eElement,
                            bBackground,
                            bImage,
                            std::move(rNewPrimitiveSequence),
                            pAnchorKey,
                            &annotIds));
                    rNewPrimitiveSequence = drawinglayer::primitive2d::Primitive2DContainer { xReference };
                }
            }
        }
        else
        {
            // page backgrounds etc should be tagged as artifacts:
            rNewPrimitiveSequence = drawinglayer::primitive2d::Primitive2DContainer {
                    new drawinglayer::primitive2d::StructureTagPrimitive2D(
                        // lies to force silly VclMetafileProcessor2D to emit NonStructElement
                        vcl::PDFWriter::Division,
                        true,
                        true,
                        std::move(rNewPrimitiveSequence))
                };
        }
    }
}

drawinglayer::primitive2d::Primitive2DContainer const & ViewObjectContact::getPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
{
    // only some of the top-level apps are any good at reliably invalidating us (e.g. writer is not)
    SdrObject* pSdrObj(mrViewContact.TryToGetSdrObject());

    if (nullptr != pSdrObj && pSdrObj->getSdrModelFromSdrObject().IsVOCInvalidationIsReliable())
    {
        if (!mxPrimitive2DSequence.empty())
            return mxPrimitive2DSequence;
    }

    // prepare new representation
    drawinglayer::primitive2d::Primitive2DContainer xNewPrimitiveSequence;

    // take care of redirectors and create new list
    ViewObjectContactRedirector* pRedirector = GetObjectContact().GetViewObjectContactRedirector();

    if(pRedirector)
    {
        pRedirector->createRedirectedPrimitive2DSequence(*this, rDisplayInfo, xNewPrimitiveSequence);
    }
    else
    {
        createPrimitive2DSequence(rDisplayInfo, xNewPrimitiveSequence);
    }

    // check and eventually embed to GridOffset transform primitive (calc only)
    if(!xNewPrimitiveSequence.empty() && GetObjectContact().supportsGridOffsets())
    {
        const basegfx::B2DVector& rGridOffset(getGridOffset());

        if(0.0 != rGridOffset.getX() || 0.0 != rGridOffset.getY())
        {
            const basegfx::B2DHomMatrix aTranslateGridOffset(
                basegfx::utils::createTranslateB2DHomMatrix(
                    rGridOffset));
            drawinglayer::primitive2d::Primitive2DReference aEmbed(
                new drawinglayer::primitive2d::TransformPrimitive2D(
                    aTranslateGridOffset,
                    std::move(xNewPrimitiveSequence)));
            xNewPrimitiveSequence = drawinglayer::primitive2d::Primitive2DContainer { aEmbed };
        }
    }

    createStructureTag(xNewPrimitiveSequence);

    // Local up-to-date checks. New list different from local one?
    // This is the important point where it gets decided if the current or the new
    // representation gets used. This is important for performance, since the
    // current representation contains possible precious decompositions. That
    // comparisons triggers exactly if something in the object visualization
    // has changed.
    // Note: That is the main reason for BasePrimitive2D::operator== at all. I
    // have alternatively tried to invalidate the local representation on object
    // change, but that is simply not reliable.
    // Note2: I did that once in aw080, the lost CWS, and it worked well enough
    // so that I could remove *all* operator== from all derivations of
    // BasePrimitive2D, so it can be done again (with the needed resources)
    if(mxPrimitive2DSequence != xNewPrimitiveSequence)
    {
        // has changed, copy content
        const_cast< ViewObjectContact* >(this)->mxPrimitive2DSequence = std::move(xNewPrimitiveSequence);

        // check for animated stuff
        const_cast< ViewObjectContact* >(this)->checkForPrimitive2DAnimations();

        // always update object range when PrimitiveSequence changes
        const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
        const_cast< ViewObjectContact* >(this)->maObjectRange = mxPrimitive2DSequence.getB2DRange(rViewInformation2D);
    }

    // return current Primitive2DContainer
    return mxPrimitive2DSequence;
}

bool ViewObjectContact::isPrimitiveVisible(const DisplayInfo& /*rDisplayInfo*/) const
{
    // default: always visible
    return true;
}

bool ViewObjectContact::isPrimitiveGhosted(const DisplayInfo& rDisplayInfo) const
{
    // default: standard check
    return (GetObjectContact().DoVisualizeEnteredGroup() && !GetObjectContact().isOutputToPrinter() && rDisplayInfo.IsGhostedDrawModeActive());
}

void ViewObjectContact::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
{
    // check model-view visibility
    if(!isPrimitiveVisible(rDisplayInfo))
        return;

    getPrimitive2DSequence(rDisplayInfo);
    if(mxPrimitive2DSequence.empty())
        return;

    // get ranges
    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
    // tdf#147164 cannot use maObjectRange here, it is unreliable
    const basegfx::B2DRange aObjectRange(mxPrimitive2DSequence.getB2DRange(rViewInformation2D));
    const basegfx::B2DRange& aViewRange(rViewInformation2D.getViewport());

    // check geometrical visibility
    bool bVisible = aViewRange.isEmpty() || aViewRange.overlaps(aObjectRange);
    if(!bVisible)
        return;

    // temporarily take over the mxPrimitive2DSequence, in case it gets invalidated while we want to iterate over it
    auto tmp = std::move(const_cast<ViewObjectContact*>(this)->mxPrimitive2DSequence);
    int nPrevCount = mnActionChangedCount;

    rVisitor.visit(tmp);

    // if we received ActionChanged() calls while walking the primitives, then leave it empty, otherwise move it back
    if (mnActionChangedCount == nPrevCount)
        const_cast<ViewObjectContact*>(this)->mxPrimitive2DSequence = std::move(tmp);
}

void ViewObjectContact::getPrimitive2DSequenceSubHierarchy(DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
{
    ViewContact& rViewContact = GetViewContact();
    const sal_uInt32 nSubHierarchyCount(rViewContact.GetObjectCount());

    for(sal_uInt32 a(0); a < nSubHierarchyCount; a++)
        rViewContact.getPrimitive2DSequenceHierarchyOfIndex(a, rDisplayInfo, GetObjectContact(), rVisitor);
}

// Support getting a GridOffset per object and view for non-linear ViewToDevice
// transformation (calc). On-demand created by delegating to the ObjectContact
// (->View) that has then all needed information
const basegfx::B2DVector& ViewObjectContact::getGridOffset() const
{
    if (GetObjectContact().supportsGridOffsets())
    {
        if (fabs(maGridOffset.getX()) > 1000.0)
        {
            // Huge offsets are a hint for error -> usually the conditions for
            // calculation have changed. E.g. - I saw errors with +/-5740, that
            // was in the environment of massive external UNO API using LO as
            // target.
            // If conditions for this calculation change, it is usually required to call
            // - ViewObjectContact::resetGridOffset(), or
            // - ObjectContact::resetAllGridOffsets() or
            // - ScDrawView::resetGridOffsetsForAllSdrPageViews()
            // as it is done e.g. when zoom changes (see ScDrawView::RecalcScale()).
            // Theoretically these resets have to be done for any precondition
            // changed that is used in the calculation of that value (see
            // ScDrawView::calculateGridOffsetForSdrObject).
            // This is not complete and would be hard to do so.
            // Since it is just a buffered value and re-calculation is not
            // expensive (linear O(n)) we can just reset suspicious values here.
            // Hopefully - when that non-linear ViewTransformation problem for
            // the calc-view gets solved one day - all this can be removed
            // again. For now, let's just reset here and force re-calculation.
            // Add a SAL_WARN to inform about this.
            SAL_WARN("svx", "Suspicious GridOffset value resetted (!)");
            const_cast<ViewObjectContact*>(this)->maGridOffset.setX(0.0);
            const_cast<ViewObjectContact*>(this)->maGridOffset.setY(0.0);
        }

        if(0.0 == maGridOffset.getX() && 0.0 == maGridOffset.getY() && GetObjectContact().supportsGridOffsets())
        {
            // create on-demand
            GetObjectContact().calculateGridOffsetForViewObjectContact(const_cast<ViewObjectContact*>(this)->maGridOffset, *this);
        }
    }

    return maGridOffset;
}

void ViewObjectContact::resetGridOffset()
{
    // reset buffered GridOffset itself
    maGridOffset.setX(0.0);
    maGridOffset.setY(0.0);

    // also reset sequence to get a re-calculation when GridOffset changes
    mxPrimitive2DSequence.clear();
    maObjectRange.reset();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
