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
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>

#include "nodetools.hxx"
#include "doctreenode.hxx"
#include "animationbasenode.hxx"
#include "delayevent.hxx"
#include "framerate.hxx"

#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <algorithm>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

AnimationBaseNode::AnimationBaseNode(
    const uno::Reference< animations::XAnimationNode >&   xNode,
    const BaseContainerNodeSharedPtr&                     rParent,
    const NodeContext&                                    rContext )
    : BaseNode( xNode, rParent, rContext ),
      mxAnimateNode( xNode, uno::UNO_QUERY_THROW ),
      maAttributeLayerHolder(),
      maSlideSize( rContext.maSlideSize ),
      mpActivity(),
      mpShape(),
      mpShapeSubset(),
      mpSubsetManager(rContext.maContext.mpSubsettableShapeManager),
      mbIsIndependentSubset( rContext.mbIsIndependentSubset )
{
    // extract native node targets
    // ===========================

    // plain shape target
    uno::Reference< drawing::XShape > xShape( mxAnimateNode->getTarget(),
                                              uno::UNO_QUERY );

    // distinguish 5 cases:
    //
    //  - plain shape target
    //  (NodeContext.mpMasterShapeSubset full set)
    //
    //  - parent-generated subset (generate an
    //  independent subset)
    //
    //  - parent-generated subset from iteration
    //  (generate a dependent subset)
    //
    //  - XShape target at the XAnimatioNode (generate
    //  a plain shape target)
    //
    //  - ParagraphTarget target at the XAnimationNode
    //  (generate an independent shape subset)
    if( rContext.mpMasterShapeSubset )
    {
        if( rContext.mpMasterShapeSubset->isFullSet() )
        {
            // case 1: plain shape target from parent
            mpShape = rContext.mpMasterShapeSubset->getSubsetShape();
        }
        else
        {
            // cases 2 & 3: subset shape
            mpShapeSubset = rContext.mpMasterShapeSubset;
        }
    }
    else
    {
        // no parent-provided shape, try to extract
        // from XAnimationNode - cases 4 and 5

        if( xShape.is() )
        {
            mpShape = lookupAttributableShape( getContext().mpSubsettableShapeManager,
                                               xShape );
        }
        else
        {
            // no shape provided. Maybe a ParagraphTarget?
            presentation::ParagraphTarget aTarget;

            if( !(mxAnimateNode->getTarget() >>= aTarget) )
                ENSURE_OR_THROW(
                    false, "could not extract any target information" );

            xShape = aTarget.Shape;

            ENSURE_OR_THROW( xShape.is(), "invalid shape in ParagraphTarget" );

            mpShape = lookupAttributableShape( getContext().mpSubsettableShapeManager,
                                               xShape );

            // NOTE: For shapes with ParagraphTarget, we ignore
            // the SubItem property. We implicitely assume that it
            // is set to ONLY_TEXT.
            OSL_ENSURE(
                mxAnimateNode->getSubItem() ==
                presentation::ShapeAnimationSubType::ONLY_TEXT ||
                mxAnimateNode->getSubItem() ==
                presentation::ShapeAnimationSubType::AS_WHOLE,
                "ParagraphTarget given, but subitem not AS_TEXT or AS_WHOLE? "
                "Make up your mind, I'll ignore the subitem." );

            // okay, found a ParagraphTarget with a valid XShape. Does the shape
            // provide the given paragraph?
            const DocTreeNode& rTreeNode(
                mpShape->getTreeNodeSupplier().getTreeNode(
                    aTarget.Paragraph,
                    DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH ) );

            // CAUTION: the creation of the subset shape
            // _must_ stay in the node constructor, since
            // Slide::prefetchShow() initializes shape
            // attributes right after animation import (or
            // the Slide class must be changed).
            mpShapeSubset.reset(
                new ShapeSubset( mpShape,
                                 rTreeNode,
                                 mpSubsetManager ));

            // Override NodeContext, and flag this node as
            // a special independent subset one. This is
            // important when applying initial attributes:
            // independent shape subsets must be setup
            // when the slide starts, since they, as their
            // name suggest, can have state independent to
            // the master shape. The following example
            // might illustrate that: a master shape has
            // no effect, one of the text paragraphs
            // within it has an appear effect. Now, the
            // respective paragraph must be invisible when
            // the slide is initially shown, and become
            // visible only when the effect starts.
            mbIsIndependentSubset = true;

            // already enable subset right here, the
            // setup of initial shape attributes of
            // course needs the subset shape
            // generated, to apply e.g. visibility
            // changes.
            mpShapeSubset->enableSubsetShape();
        }
    }
}

void AnimationBaseNode::dispose()
{
    if (mpActivity) {
        mpActivity->dispose();
        mpActivity.reset();
    }

    maAttributeLayerHolder.reset();
    mxAnimateNode.clear();
    mpShape.reset();
    mpShapeSubset.reset();

    BaseNode::dispose();
}

bool AnimationBaseNode::init_st()
{
    // if we've still got an old activity lying around, dispose it:
    if (mpActivity) {
        mpActivity->dispose();
        mpActivity.reset();
    }

    // note: actually disposing the activity too early might cause problems,
    // because on dequeued() it calls endAnimation(pAnim->end()), thus ending
    // animation _after_ last screen update.
    // review that end() is properly called (which calls endAnimation(), too).

    try {
        // TODO(F2): For restart functionality, we must regenerate activities,
        // since they are not able to reset their state (or implement _that_)
        mpActivity = createActivity();
    }
    catch (uno::Exception const&) {
        OSL_FAIL( rtl::OUStringToOString(
                        comphelper::anyToString(cppu::getCaughtException()),
                        RTL_TEXTENCODING_UTF8 ) );
        // catch and ignore. We later handle empty activities, but for
        // other nodes to function properly, the core functionality of
        // this node must remain up and running.
    }
    return true;
}

bool AnimationBaseNode::resolve_st()
{
    // enable shape subset for automatically generated
    // subsets. Independent subsets are already setup
    // during construction time. Doing it only here
    // saves us a lot of sprites and shapes lying
    // around. This is especially important for
    // character-wise iterations, since the shape
    // content (e.g. thousands of characters) would
    // otherwise be painted character-by-character.
    if (isDependentSubsettedShape() && mpShapeSubset) {
        mpShapeSubset->enableSubsetShape();
    }
    return true;
}

void AnimationBaseNode::activate_st()
{
    // create new attribute layer
    maAttributeLayerHolder.createAttributeLayer( getShape() );

    ENSURE_OR_THROW( maAttributeLayerHolder.get(),
                      "Could not generate shape attribute layer" );

    // TODO(Q2): This affects the way mpActivity
    // works, but is performed here because of
    // locality (we're fiddling with the additive mode
    // here, anyway, and it's the only place where we
    // do). OTOH, maybe the complete additive mode
    // setup should be moved to the activities.

    // for simple by-animations, the SMIL spec
    // requires us to emulate "0,by-value" value list
    // behaviour, with additive mode forced to "sum",
    // no matter what the input is
    // (http://www.w3.org/TR/smil20/animation.html#adef-by).
    if( mxAnimateNode->getBy().hasValue() &&
        !mxAnimateNode->getTo().hasValue() &&
        !mxAnimateNode->getFrom().hasValue() )
    {
        // force attribute mode to REPLACE (note the
        // subtle discrepancy to the paragraph above,
        // where SMIL requires SUM. This is internally
        // handled by the FromToByActivity, and is
        // because otherwise DOM values would not be
        // handled correctly: the activity cannot
        // determine whether an
        // Activity::getUnderlyingValue() yields the
        // DOM value, or already a summed-up conglomerate)
        //
        // Note that this poses problems with our
        // hybrid activity duration (time or min number of frames),
        // since if activities
        // exceed their duration, wrong 'by' start
        // values might arise ('Laser effect')
        maAttributeLayerHolder.get()->setAdditiveMode(
            animations::AnimationAdditiveMode::REPLACE );
    }
    else
    {
        // apply additive mode to newly created Attribute layer
        maAttributeLayerHolder.get()->setAdditiveMode(
            mxAnimateNode->getAdditive() );
    }

    // fake normal animation behaviour, even if we
    // show nothing.  This is the appropriate way to
    // handle errors on Activity generation, because
    // maybe all other effects on the slide are
    // correctly initialized (but won't run, if we
    // signal an error here)
    if (mpActivity) {
        // supply Activity (and the underlying Animation) with
        // it's AttributeLayer, to perform the animation on
        mpActivity->setTargets( getShape(), maAttributeLayerHolder.get() );

        // add to activities queue
        getContext().mrActivitiesQueue.addActivity( mpActivity );
    }
    else {
        // Actually, DO generate the event for empty activity,
        // to keep the chain of animations running
        BaseNode::scheduleDeactivationEvent();
    }
}

void AnimationBaseNode::deactivate_st( NodeState eDestState )
{
    if (eDestState == FROZEN) {
        if (mpActivity)
            mpActivity->end();
    }

    if (isDependentSubsettedShape()) {
        // for dependent subsets, remove subset shape
        // from layer, re-integrate subsetted part
        // back into original shape. For independent
        // subsets, we cannot make any assumptions
        // about subset attribute state relative to
        // master shape, thus, have to keep it. This
        // will effectively re-integrate the subsetted
        // part into the original shape (whose
        // animation will hopefully have ended, too)

        // this statement will save a whole lot of
        // sprites for iterated text effects, since
        // those sprites will only exist during the
        // actual lifetime of the effects
        if (mpShapeSubset) {
            mpShapeSubset->disableSubsetShape();
        }
    }

    if (eDestState == ENDED) {

        // no shape anymore, no layer needed:
        maAttributeLayerHolder.reset();

        if (! isDependentSubsettedShape()) {

            // for all other shapes, removing the
            // attribute layer quite possibly changes
            // shape display. Thus, force update
            AttributableShapeSharedPtr const pShape( getShape() );

            // don't anybody dare to check against
            // pShape->isVisible() here, removing the
            // attribute layer might actually make the
            // shape invisible!
            getContext().mpSubsettableShapeManager->notifyShapeUpdate( pShape );
        }

        if (mpActivity) {
            // kill activity, if still running
            mpActivity->dispose();
            mpActivity.reset();
        }
    }
}

bool AnimationBaseNode::hasPendingAnimation() const
{
    // TODO(F1): This might not always be true. Are there 'inactive'
    // animation nodes?
    return true;
}

#if defined(VERBOSE) && defined(DBG_UTIL)
void AnimationBaseNode::showState() const
{
    BaseNode::showState();

    VERBOSE_TRACE( "AnimationBaseNode info: independent subset=%s",
                   mbIsIndependentSubset ? "y" : "n" );
}
#endif

ActivitiesFactory::CommonParameters
AnimationBaseNode::fillCommonParameters() const
{
    double nDuration = 0.0;

    // TODO(F3): Duration/End handling is barely there
    if( !(mxAnimateNode->getDuration() >>= nDuration) ) {
        mxAnimateNode->getEnd() >>= nDuration; // Wah.
    }

    // minimal duration we fallback to (avoid 0 here!)
    nDuration = ::std::max( 0.001, nDuration );

    const bool bAutoReverse( mxAnimateNode->getAutoReverse() );

    boost::optional<double> aRepeats;
    double nRepeats = 0;
    if( (mxAnimateNode->getRepeatCount() >>= nRepeats) ) {
        aRepeats.reset( nRepeats );
    }
    else {
        if( (mxAnimateNode->getRepeatDuration() >>= nRepeats) ) {
            // when repeatDuration is given,
            // autoreverse does _not_ modify the
            // active duration. Thus, calc repeat
            // count with already adapted simple
            // duration (twice the specified duration)

            // convert duration back to repeat counts
            if( bAutoReverse )
                aRepeats.reset( nRepeats / (2.0 * nDuration) );
            else
                aRepeats.reset( nRepeats / nDuration );
        }
        else {
            // no double value for both values - Timing::INDEFINITE?
            animations::Timing eTiming;

            if( !(mxAnimateNode->getRepeatDuration() >>= eTiming) ||
                eTiming != animations::Timing_INDEFINITE )
            {
                if( !(mxAnimateNode->getRepeatCount() >>= eTiming) ||
                    eTiming != animations::Timing_INDEFINITE )
                {
                    // no indefinite timing, no other values given -
                    // use simple run, i.e. repeat of 1.0
                    aRepeats.reset( 1.0 );
                }
            }
        }
    }

    // calc accel/decel:
    double nAcceleration = 0.0;
    double nDeceleration = 0.0;
    BaseNodeSharedPtr const pSelf( getSelf() );
    for ( boost::shared_ptr<BaseNode> pNode( pSelf );
          pNode; pNode = pNode->getParentNode() )
    {
        uno::Reference<animations::XAnimationNode> const xAnimationNode(
            pNode->getXAnimationNode() );
        nAcceleration = std::max( nAcceleration,
                                  xAnimationNode->getAcceleration() );
        nDeceleration = std::max( nDeceleration,
                                  xAnimationNode->getDecelerate() );
    }

    EventSharedPtr pEndEvent;
    if (pSelf) {
        pEndEvent = makeEvent(
            boost::bind( &AnimationNode::deactivate, pSelf ),
            "AnimationBaseNode::deactivate");
    }

    // Calculate the minimum frame count that depends on the duration and
    // the minimum frame count.
    const sal_Int32 nMinFrameCount (basegfx::clamp<sal_Int32>(
        basegfx::fround(nDuration * FrameRate::MinimumFramesPerSecond), 1, 10));

    return ActivitiesFactory::CommonParameters(
        pEndEvent,
        getContext().mrEventQueue,
        getContext().mrActivitiesQueue,
        nDuration,
        nMinFrameCount,
        bAutoReverse,
        aRepeats,
        nAcceleration,
        nDeceleration,
        getShape(),
        getSlideSize());
}

AttributableShapeSharedPtr AnimationBaseNode::getShape() const
{
    // any subsetting at all?
    if (mpShapeSubset)
        return mpShapeSubset->getSubsetShape();
    else
        return mpShape; // nope, plain shape always
}

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
