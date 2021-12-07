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
    drawinglayer::primitive2d::Primitive2DContainer extractPrimitive2DSequence() const { return std::move(maPrimitive2DSequence); }
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
    mbLazyInvalidate(false)
{
    // make the ViewContact remember me
    mrViewContact.AddViewObjectContact(*this);

    // make the ObjectContact remember me
    mrObjectContact.AddViewObjectContact(*this);
}

ViewObjectContact::~ViewObjectContact()
{
    // invalidate in view
    if(!maObjectRange.isEmpty())
    {
        GetObjectContact().InvalidatePartOfView(maObjectRange);
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
            const drawinglayer::primitive2d::Primitive2DContainer xSequence(getPrimitive2DSequence(aDisplayInfo));

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
    if(mbLazyInvalidate)
        return;

    // set local flag
    mbLazyInvalidate = true;

    // force ObjectRange
    getObjectRange();

    if(!maObjectRange.isEmpty())
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

void ViewObjectContact::triggerLazyInvalidate()
{
    if(!mbLazyInvalidate)
        return;

    // reset flag
    mbLazyInvalidate = false;

    // force ObjectRange
    getObjectRange();

    if(!maObjectRange.isEmpty())
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

drawinglayer::primitive2d::Primitive2DContainer const & ViewObjectContact::getPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
{
    if (!mxPrimitive2DSequence.empty())
        return mxPrimitive2DSequence;

    /**
    This method is weird because
    (1) we have to re-walk the primitive tree because the flushing is unreliable
    (2) we cannot just always use the new data because the old data has cached bitmaps in it e.g. see the documents in tdf#104878
    */

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

    // local up-to-date checks. New list different from local one?
    if(mxPrimitive2DSequence == xNewPrimitiveSequence)
        return mxPrimitive2DSequence;

    // has changed, copy content
    const_cast< ViewObjectContact* >(this)->mxPrimitive2DSequence = std::move(xNewPrimitiveSequence);

    // check for animated stuff
    const_cast< ViewObjectContact* >(this)->checkForPrimitive2DAnimations();

    // always update object range when PrimitiveSequence changes
    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
    const_cast< ViewObjectContact* >(this)->maObjectRange = mxPrimitive2DSequence.getB2DRange(rViewInformation2D);

    // check and eventually embed to GridOffset transform primitive
    if(GetObjectContact().supportsGridOffsets())
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
                    std::move(const_cast< ViewObjectContact* >(this)->mxPrimitive2DSequence)));

            // Set values at local data. So for now, the mechanism is to reset some of the
            // defining things (mxPrimitive2DSequence, maGridOffset) and re-create the
            // buffered data (including maObjectRange). It *could* be changed to keep
            // the unmodified PrimitiveSequence and only update the GridOffset, but this
            // would require a 2nd instance of maObjectRange and mxPrimitive2DSequence. I
            // started doing so, but it just makes the code more complicated. For now,
            // just allow re-creation of the PrimitiveSequence (and removing buffered
            // decomposed content of it). May be optimized, though. OTOH it only happens
            // in calc which traditionally does not have a huge amount of DrawObjects anyways.
            const_cast< ViewObjectContact* >(this)->mxPrimitive2DSequence = drawinglayer::primitive2d::Primitive2DContainer { aEmbed };
            const_cast< ViewObjectContact* >(this)->maObjectRange.transform(aTranslateGridOffset);
        }
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

    drawinglayer::primitive2d::Primitive2DContainer xRetval = getPrimitive2DSequence(rDisplayInfo);
    if(xRetval.empty())
        return;

    // get ranges
    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
    const basegfx::B2DRange aObjectRange(xRetval.getB2DRange(rViewInformation2D));
    const basegfx::B2DRange& aViewRange(rViewInformation2D.getViewport());

    // check geometrical visibility
    bool bVisible = aViewRange.isEmpty() || aViewRange.overlaps(aObjectRange);
    if(!bVisible)
        return;

    rVisitor.visit(std::move(xRetval));
}

void ViewObjectContact::getPrimitive2DSequenceSubHierarchy(DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
{
    const sal_uInt32 nSubHierarchyCount(GetViewContact().GetObjectCount());

    for(sal_uInt32 a(0); a < nSubHierarchyCount; a++)
    {
        const ViewObjectContact& rCandidate(GetViewContact().GetViewContact(a).GetViewObjectContact(GetObjectContact()));

        rCandidate.getPrimitive2DSequenceHierarchy(rDisplayInfo, rVisitor);
    }
}

// Support getting a GridOffset per object and view for non-linear ViewToDevice
// transformation (calc). On-demand created by delegating to the ObjectContact
// (->View) that has then all needed information
const basegfx::B2DVector& ViewObjectContact::getGridOffset() const
{
    if(0.0 == maGridOffset.getX() && 0.0 == maGridOffset.getY() && GetObjectContact().supportsGridOffsets())
    {
        // create on-demand
        GetObjectContact().calculateGridOffsetForViewOjectContact(const_cast<ViewObjectContact*>(this)->maGridOffset, *this);
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
