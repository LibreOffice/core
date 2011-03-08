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

#include "precompiled_slideshow.hxx"

#include "effectrewinder.hxx"
#include "eventqueue.hxx"
#include "usereventqueue.hxx"
#include "mouseeventhandler.hxx"
#include "animationnodes/basecontainernode.hxx"
#include "delayevent.hxx"

#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star;

namespace slideshow { namespace internal {


namespace {

class RewinderEventHandler : public EventHandler
{
public:
    typedef ::boost::function<bool(void)> Action;
    RewinderEventHandler (const Action& rAction) : maAction(rAction) {}
    virtual ~RewinderEventHandler (void) {}
private:
    const Action maAction;
    virtual bool handleEvent (void) { return maAction(); }
};



class RewinderAnimationEventHandler : public AnimationEventHandler
{
public:
    typedef ::boost::function<bool(const AnimationNodeSharedPtr& rpNode)> Action;
    RewinderAnimationEventHandler (const Action& rAction) : maAction(rAction) {}
    virtual ~RewinderAnimationEventHandler (void) {}
private:
    const Action maAction;
    virtual bool handleAnimationEvent (const AnimationNodeSharedPtr& rpNode)
        { return maAction(rpNode); }
};



} // end of anonymous namespace


//----- EffectRewinder --------------------------------------------------------------

EffectRewinder::EffectRewinder (
    EventMultiplexer& rEventMultiplexer,
    EventQueue& rEventQueue,
    UserEventQueue& rUserEventQueue)
    : mrEventMultiplexer(rEventMultiplexer),
      mrEventQueue(rEventQueue),
      mrUserEventQueue(rUserEventQueue),
      mpSlideStartHandler(),
      mpSlideEndHandler(),
      mpAnimationStartHandler(),
      mnMainSequenceEffectCount(0),
      mpAsynchronousRewindEvent(),
      mxCurrentAnimationRootNode(),
      mbNonUserTriggeredMainSequenceEffectSeen(false)
{
    initialize();
}




void EffectRewinder::initialize (void)
{
    // Add some event handlers so that we are informed when
    // a) an animation is started (we then check whether that belongs to a
    // main sequence effect and if so, increase the respective counter),
    // b,c) a slide was started or ended (in which case the effect counter
    // is reset.

    mpAnimationStartHandler.reset(
        new RewinderAnimationEventHandler(
            ::boost::bind(&EffectRewinder::notifyAnimationStart, this, _1)));
    mrEventMultiplexer.addAnimationStartHandler(mpAnimationStartHandler);

    mpSlideStartHandler.reset(
        new RewinderEventHandler(
            ::boost::bind(&EffectRewinder::resetEffectCount, this)));
    mrEventMultiplexer.addSlideStartHandler(mpSlideStartHandler);

    mpSlideEndHandler.reset(
        new RewinderEventHandler(
            ::boost::bind(&EffectRewinder::resetEffectCount, this)));
    mrEventMultiplexer.addSlideEndHandler(mpSlideEndHandler);
}




EffectRewinder::~EffectRewinder (void)
{
    dispose();
}




void EffectRewinder::dispose (void)
{
    if (mpAsynchronousRewindEvent)
    {
        mpAsynchronousRewindEvent->dispose();
        mpAsynchronousRewindEvent.reset();
    }

    if (mpAnimationStartHandler)
    {
        mrEventMultiplexer.removeAnimationStartHandler(mpAnimationStartHandler);
        mpAnimationStartHandler.reset();
    }

    if (mpSlideStartHandler)
    {
        mrEventMultiplexer.removeSlideStartHandler(mpSlideStartHandler);
        mpSlideStartHandler.reset();
    }

    if (mpSlideEndHandler)
    {
        mrEventMultiplexer.removeSlideEndHandler(mpSlideEndHandler);
        mpSlideEndHandler.reset();
    }
}




void EffectRewinder::setRootAnimationNode (
    const uno::Reference<animations::XAnimationNode>& xRootNode)
{
    mxCurrentAnimationRootNode = xRootNode;
}




bool EffectRewinder::rewind (
    const ::boost::shared_ptr<ScreenUpdater::UpdateLock>& rpPaintLock,
    const ::boost::function<void(void)>& rSlideRewindFunctor,
    const ::boost::function<void(void)>& rPreviousSlideFunctor)
{
    mpPaintLock = rpPaintLock;

    // Do not allow nested rewinds.
    if (mpAsynchronousRewindEvent)
    {
        OSL_ASSERT( ! mpAsynchronousRewindEvent);
        return false;
    }

    // Abort (and skip over the rest of) any currently active animation.
    mrUserEventQueue.callSkipEffectEventHandler();
    mrEventQueue.forceEmpty();

    const int nSkipCount (mnMainSequenceEffectCount - 1);
    if (nSkipCount < 0)
    {
        if ( ! rPreviousSlideFunctor)
        {
            OSL_ASSERT(rPreviousSlideFunctor);
            return false;
        }

        // No main sequence effects to rewind on the current slide.
        // Go back to the previous slide.
        mpAsynchronousRewindEvent = makeEvent(
            ::boost::bind(
                &EffectRewinder::asynchronousRewindToPreviousSlide,
                this,
                rPreviousSlideFunctor),
            "EffectRewinder::asynchronousRewindToPreviousSlide");
    }
    else
    {
        // The actual rewinding is done asynchronously so that we can safely
        // call other methods.
        mpAsynchronousRewindEvent = makeEvent(
            ::boost::bind(
                &EffectRewinder::asynchronousRewind,
                this,
                nSkipCount,
                true,
                rSlideRewindFunctor),
            "EffectRewinder::asynchronousRewind");
    }

    if (mpAsynchronousRewindEvent)
        mrEventQueue.addEvent(mpAsynchronousRewindEvent);

    return mpAsynchronousRewindEvent.get()!=NULL;
}




void EffectRewinder::skipAllMainSequenceEffects (void)
{
    // Do not allow nested rewinds.
    if (mpAsynchronousRewindEvent)
    {
        OSL_ASSERT(!mpAsynchronousRewindEvent);
        return;
    }

    const int nTotalMainSequenceEffectCount (countMainSequenceEffects());
    mpAsynchronousRewindEvent = makeEvent(
        ::boost::bind(
            &EffectRewinder::asynchronousRewind,
            this,
            nTotalMainSequenceEffectCount,
            false,
            ::boost::function<void(void)>()),
        "EffectRewinder::asynchronousRewind");
    mrEventQueue.addEvent(mpAsynchronousRewindEvent);
}




sal_Int32 EffectRewinder::countMainSequenceEffects (void)
{
    // Determine the number of main sequence effects.
    sal_Int32 nMainSequenceNodeCount (0);

    ::std::queue<uno::Reference<animations::XAnimationNode> > aNodeQueue;
    aNodeQueue.push(mxCurrentAnimationRootNode);
    while ( ! aNodeQueue.empty())
    {
        const uno::Reference<animations::XAnimationNode> xNode (aNodeQueue.front());
        aNodeQueue.pop();

        // Does the current node belong to the main sequence?
        if (xNode.is())
        {
            animations::Event aEvent;
            if (xNode->getBegin() >>= aEvent)
                if (aEvent.Trigger == animations::EventTrigger::ON_NEXT)
                    ++nMainSequenceNodeCount;
        }

        // If the current node is a container then prepare its children for investigation.
        uno::Reference<container::XEnumerationAccess> xEnumerationAccess (xNode, uno::UNO_QUERY);
        if (xEnumerationAccess.is())
        {
            uno::Reference<container::XEnumeration> xEnumeration (
                xEnumerationAccess->createEnumeration());
            if (xEnumeration.is())
                while (xEnumeration->hasMoreElements())
                {
                    aNodeQueue.push(
                        uno::Reference<animations::XAnimationNode>(
                            xEnumeration->nextElement(), uno::UNO_QUERY));
                }
        }
    }

    return nMainSequenceNodeCount;

    //    // Skip all main sequence nodes.
    //    SkipSomeMainSequenceEffects(nMainSequenceNodeCount);
}




void EffectRewinder::skipSomeMainSequenceEffects (sal_Int32 nSkipCount)
{
    while (--nSkipCount >= 0)
        skipSingleMainSequenceEffects();
}




void EffectRewinder::skipSingleMainSequenceEffects (void)
{
    // This basically just starts the next effect and then skips over its
    // animation.
    mrEventMultiplexer.notifyNextEffect();
    mrEventQueue.forceEmpty();
    mrUserEventQueue.callSkipEffectEventHandler();
    mrEventQueue.forceEmpty();
}




bool EffectRewinder::resetEffectCount (void)
{
    mnMainSequenceEffectCount = 0;
    return false;
}




bool EffectRewinder::notifyAnimationStart (const AnimationNodeSharedPtr& rpNode)
{
    // This notification is only relevant for us when the rpNode belongs to
    // the main sequence.
    BaseNodeSharedPtr pBaseNode (::boost::dynamic_pointer_cast<BaseNode>(rpNode));
    if ( ! pBaseNode)
        return false;

    BaseContainerNodeSharedPtr pParent (pBaseNode->getParentNode());
    if ( ! (pParent && pParent->isMainSequenceRootNode()))
        return false;

    // This notification is only relevant for us when the effect is user
    // triggered.
    bool bIsUserTriggered (false);

    Reference<animations::XAnimationNode> xNode (rpNode->getXAnimationNode());
    if (xNode.is())
    {
        animations::Event aEvent;
        if ((xNode->getBegin() >>= aEvent))
            bIsUserTriggered = (aEvent.Trigger == animations::EventTrigger::ON_NEXT);
    }

    if (bIsUserTriggered)
        ++mnMainSequenceEffectCount;
    else
        mbNonUserTriggeredMainSequenceEffectSeen = true;

    return false;
}




void EffectRewinder::asynchronousRewind (
    sal_Int32 nEffectCount,
    const bool bRedisplayCurrentSlide,
    const boost::function<void(void)>& rSlideRewindFunctor)
{
    OSL_ASSERT(mpAsynchronousRewindEvent);

    if (bRedisplayCurrentSlide)
    {
        mpPaintLock->Activate();
        // Re-display the current slide.
        if (rSlideRewindFunctor)
            rSlideRewindFunctor();
        mpAsynchronousRewindEvent = makeEvent(
            ::boost::bind(
                &EffectRewinder::asynchronousRewind,
                this,
                nEffectCount,
                false,
                rSlideRewindFunctor),
            "EffectRewinder::asynchronousRewind");
        mrEventQueue.addEvent(mpAsynchronousRewindEvent);
    }
    else
    {
        // Process initial events and skip any animations that are started
        // when the slide is shown.
        mbNonUserTriggeredMainSequenceEffectSeen = false;
        mrEventQueue.forceEmpty();
        if (mbNonUserTriggeredMainSequenceEffectSeen)
        {
            mrUserEventQueue.callSkipEffectEventHandler();
            mrEventQueue.forceEmpty();
        }

        while (--nEffectCount >= 0)
            skipSingleMainSequenceEffects();

        mpAsynchronousRewindEvent.reset();
        mpPaintLock.reset();
    }
}




void EffectRewinder::asynchronousRewindToPreviousSlide (
    const ::boost::function<void(void)>& rSlideRewindFunctor)
{
    OSL_ASSERT(mpAsynchronousRewindEvent);

    mpAsynchronousRewindEvent.reset();
    rSlideRewindFunctor();
}




} } // end of namespace ::slideshow::internal

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
