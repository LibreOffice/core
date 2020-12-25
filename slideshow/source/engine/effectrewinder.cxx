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


#include "effectrewinder.hxx"
#include <eventqueue.hxx>
#include <usereventqueue.hxx>
#include <mouseeventhandler.hxx>
#include <basecontainernode.hxx>
#include <delayevent.hxx>
#include <tools.hxx>

#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <officecfg/Office/Canvas.hxx>

using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star;

namespace slideshow { namespace internal {


namespace {

class RewinderEventHandler : public EventHandler
{
public:
    typedef ::std::function<bool ()> Action;
    explicit RewinderEventHandler (const Action& rAction) : maAction(rAction) {}

private:
    const Action maAction;
    virtual bool handleEvent() override { return maAction(); }
};


class RewinderAnimationEventHandler : public AnimationEventHandler
{
public:
    typedef ::std::function<bool (const AnimationNodeSharedPtr& rpNode)> Action;
    explicit RewinderAnimationEventHandler (const Action& rAction) : maAction(rAction) {}

private:
    const Action maAction;
    virtual bool handleAnimationEvent (const AnimationNodeSharedPtr& rpNode) override
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
      mxCurrentSlide(),
      mbNonUserTriggeredMainSequenceEffectSeen(false)
{
    initialize();
}


void EffectRewinder::initialize()
{
    // Add some event handlers so that we are informed when
    // a) an animation is started (we then check whether that belongs to a
    // main sequence effect and if so, increase the respective counter),
    // b,c) a slide was started or ended (in which case the effect counter
    // is reset.

    mpAnimationStartHandler.reset(
        new RewinderAnimationEventHandler(
            [this]( const AnimationNodeSharedPtr& pNode)
            { return this->notifyAnimationStart( pNode ); } ) );
    mrEventMultiplexer.addAnimationStartHandler(mpAnimationStartHandler);

    mpSlideStartHandler.reset(
        new RewinderEventHandler(
            [this]() { return this->resetEffectCount(); } ) );
    mrEventMultiplexer.addSlideStartHandler(mpSlideStartHandler);

    mpSlideEndHandler.reset(
        new RewinderEventHandler(
            [this]() { return this->resetEffectCount(); } ) );
    mrEventMultiplexer.addSlideEndHandler(mpSlideEndHandler);
}


EffectRewinder::~EffectRewinder()
{
    dispose();
}


void EffectRewinder::dispose()
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

void EffectRewinder::setCurrentSlide (
    const uno::Reference<drawing::XDrawPage>& xSlide)
{
    mxCurrentSlide = xSlide;
}

bool EffectRewinder::rewind (
    const ::std::shared_ptr<ScreenUpdater::UpdateLock>& rpPaintLock,
    const ::std::function<void ()>& rSlideRewindFunctor,
    const ::std::function<void ()>& rPreviousSlideFunctor)
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
            ::std::bind(
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
            ::std::bind(
                &EffectRewinder::asynchronousRewind,
                this,
                nSkipCount,
                true,
                rSlideRewindFunctor),
            "EffectRewinder::asynchronousRewind");
    }

    if (mpAsynchronousRewindEvent)
        mrEventQueue.addEvent(mpAsynchronousRewindEvent);

    return mpAsynchronousRewindEvent.get()!=nullptr;
}


void EffectRewinder::skipAllMainSequenceEffects()
{
    // Do not allow nested rewinds.
    if (mpAsynchronousRewindEvent)
    {
        OSL_ASSERT(!mpAsynchronousRewindEvent);
        return;
    }

    const int nTotalMainSequenceEffectCount (countMainSequenceEffects());
    mpAsynchronousRewindEvent = makeEvent(
        ::std::bind(
            &EffectRewinder::asynchronousRewind,
            this,
            nTotalMainSequenceEffectCount,
            false,
            ::std::function<void ()>()),
        "EffectRewinder::asynchronousRewind");
    mrEventQueue.addEvent(mpAsynchronousRewindEvent);
}


sal_Int32 EffectRewinder::countMainSequenceEffects()
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
}


void EffectRewinder::skipSingleMainSequenceEffects()
{
    // This basically just starts the next effect and then skips over its
    // animation.
    mrEventMultiplexer.notifyNextEffect();
    mrEventQueue.forceEmpty();
    mrUserEventQueue.callSkipEffectEventHandler();
    mrEventQueue.forceEmpty();
}


bool EffectRewinder::resetEffectCount()
{
    mnMainSequenceEffectCount = 0;
    return false;
}

bool EffectRewinder::hasBlockedAnimation( const css::uno::Reference<css::animations::XAnimationNode>& xNode)
{
    bool isShapeTarget = false;;
    OUString preset_id;
    OUString preset_sub_type;
    OUString preset_property;

    if (xNode->getUserData().getLength())
    {
        for(int i = 0; i < xNode->getUserData().getLength(); i++)
        {
            if(xNode->getUserData()[i].Name == "preset-id")
                xNode->getUserData()[i].Value >>= preset_id;
            if(xNode->getUserData()[i].Name == "preset-sub-type")
                xNode->getUserData()[i].Value >>= preset_sub_type;
            if(xNode->getUserData()[i].Name == "preset-property")
                xNode->getUserData()[i].Value >>= preset_property;
        }
    }

    uno::Reference<container::XEnumerationAccess> xEnumerationAccess (xNode, uno::UNO_QUERY);
    if (xEnumerationAccess.is())
    {
        uno::Reference<container::XEnumeration> xEnumeration (
            xEnumerationAccess->createEnumeration());
        if (xEnumeration.is())
            while (xEnumeration->hasMoreElements())
            {
                uno::Reference<animations::XAnimationNode> xNext (xEnumeration->nextElement(), uno::UNO_QUERY);
                uno::Reference<animations::XAnimate> xAnimate( xNext, uno::UNO_QUERY );

                if(xAnimate.is())
                {
                    uno::Reference< drawing::XShape > xShape( xAnimate->getTarget(), uno::UNO_QUERY );

                    if (xShape.is() || xAnimate->getTarget().getValueType() == cppu::UnoType<void>::get())
                        isShapeTarget=true;
                }
            }
    }

    if(isShapeTarget &&
       ((preset_id == "ooo-entrance-zoom" && preset_sub_type == "in") || // Entrance Zoom In
        (preset_id == "ooo-entrance-swivel" )                         || // Entrance Swivel
        (preset_id == "ooo-entrance-spiral-in")                       || // Entrance Spiral-In
        (preset_id == "ooo-entrance-stretchy")))                         // Entrance Stretchy
        return true;

    return false;
}

bool EffectRewinder::notifyAnimationStart (const AnimationNodeSharedPtr& rpNode)
{
    Reference<animations::XAnimationNode> xNode (rpNode->getXAnimationNode());

    if( xNode.is() &&
        !officecfg::Office::Canvas::ForceSafeServiceImpl::get() &&
        hasBlockedAnimation(xNode) )
        skipSingleMainSequenceEffects();

    // This notification is only relevant for us when the rpNode belongs to
    // the main sequence.
    BaseNodeSharedPtr pBaseNode (::std::dynamic_pointer_cast<BaseNode>(rpNode));
    if ( ! pBaseNode)
        return false;

    BaseContainerNodeSharedPtr pParent (pBaseNode->getParentNode());
    if ( ! (pParent && pParent->isMainSequenceRootNode()))
        return false;

    // This notification is only relevant for us when the effect is user
    // triggered.
    bool bIsUserTriggered (false);

    if (xNode.is())
    {
        animations::Event aEvent;
        if (xNode->getBegin() >>= aEvent)
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
    const std::function<void ()>& rSlideRewindFunctor)
{
    OSL_ASSERT(mpAsynchronousRewindEvent);

    if (bRedisplayCurrentSlide)
    {
        mpPaintLock->Activate();
        // Re-display the current slide.
        if (rSlideRewindFunctor)
            rSlideRewindFunctor();
        mpAsynchronousRewindEvent = makeEvent(
            ::std::bind(
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

        uno::Reference< beans::XPropertySet > xPropSet( mxCurrentSlide, uno::UNO_QUERY );
        sal_Int32 nChange(0);

        if( xPropSet.is())
            getPropertyValue( nChange, xPropSet, "Change");

        if (!nChange)
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
    const ::std::function<void ()>& rSlideRewindFunctor)
{
    OSL_ASSERT(mpAsynchronousRewindEvent);

    mpAsynchronousRewindEvent.reset();
    rSlideRewindFunctor();
}


} } // end of namespace ::slideshow::internal

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
