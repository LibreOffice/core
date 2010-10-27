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

#ifndef INCLUDED_SLIDESHOW_EFFECT_REWINDER_HXX
#define INCLUDED_SLIDESHOW_EFFECT_REWINDER_HXX

#include "animationnode.hxx"
#include "eventhandler.hxx"
#include "animationeventhandler.hxx"
#include "event.hxx"
#include "screenupdater.hxx"

#include <com/sun/star/presentation/XSlideShow.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <vector>

namespace css = ::com::sun::star;

namespace slideshow { namespace internal {

class EventMultiplexer;
class EventQueue;
class UserEventQueue;

/** Rewind single effects of the main effect sequence.  A rewind is
    initiated by calling the Rewind() method.  Part of the processing is
    done asynchronously.  Multiple EventQueue::update() calls may be
    necessary to finish a rewind.

    Remember to call SetRootAnimationNode() when switching to a different
    slide so that the EffectRewinder can determine the number of main
    sequence effects.
*/
class EffectRewinder
{
public:
    EffectRewinder (
        EventMultiplexer& rEventMultiplexer,
        EventQueue& rEventQueue,
        UserEventQueue& rUserEventQueue);
    ~EffectRewinder (void);

    /** Call Dispose() before the ownder of an EffectRewinder object dies so
        that the EffectRewinder can release all references to the owner.

    */
    void dispose (void);

    /** Store the root node of the animation tree.  It is used in
        CountMainSequenceEffects() to count the number of main sequence
        effects (or effect groups.)
    */
    void setRootAnimationNode (
        const css::uno::Reference<css::animations::XAnimationNode>& xRootNode);

    /** Rewind one effect of the main effect sequence.  When the current
        slide has not effects or no main sequence effect has yet been played
        then switch to the previous slide and replay all of its main
        sequence effects.
        The caller has to pass two functors that redisplay the current slide
        or switch to the previous slide so that it does not have to expose
        its internals to us.  Only one of the two functors is called.
        @param rpPaintLock
            This paint lock is released after the whole asynchronous
            procoess  of rewinding the current effect is completed.  It
            prevents intermediate repaints  that would show partial replay
            of effects.
        @param rSlideRewindFunctor
            This functor is called when the current slide is to be
            redisplayed.  When it is called then the other functor is not
            called.
        @param rPreviousSlideFunctor
            This functor is called to switch to the previous slide.  When it
            is called then the other functor is not called.
    */
    bool rewind (
        const ::boost::shared_ptr<ScreenUpdater::UpdateLock>& rpPaintLock,
        const ::boost::function<void(void)>& rSlideRewindFunctor,
        const ::boost::function<void(void)>& rPreviousSlideFunctor);

    /** Call this method after gotoPreviousEffect() triggered a slide change
        to the previous slide.
    */
    void skipAllMainSequenceEffects (void);

private:
    EventMultiplexer& mrEventMultiplexer;
    EventQueue& mrEventQueue;
    UserEventQueue& mrUserEventQueue;

    EventHandlerSharedPtr mpSlideStartHandler;
    EventHandlerSharedPtr mpSlideEndHandler;
    AnimationEventHandlerSharedPtr mpAnimationStartHandler;

    /** The number off main sequence effects so far.
    */
    sal_Int32 mnMainSequenceEffectCount;

    /** This is the currently scheduled event that executes the asynchronous
        part of the effect rewinding.  It is also used as flag that prevents
        nested rewinds.
    */
    EventSharedPtr mpAsynchronousRewindEvent;

    css::uno::Reference<css::animations::XAnimationNode> mxCurrentAnimationRootNode;
    ::boost::shared_ptr<ScreenUpdater::UpdateLock> mpPaintLock;

    bool mbNonUserTriggeredMainSequenceEffectSeen;

    void initialize (void);

    bool resetEffectCount (void);
    /** Called by listeners when an animation (not necessarily of a main
        sequence effect) starts.
    */
    bool notifyAnimationStart (const AnimationNodeSharedPtr& rpNode);

    /** Count the number of effects (or effect groups) in the main effect
        sequence.
    */
    sal_Int32 countMainSequenceEffects (void);

    /** Skip the next main sequence effect.
    */
    void skipSingleMainSequenceEffects (void);

    /** Skip the specified number of main sequence effects.
    */
    void skipSomeMainSequenceEffects (const sal_Int32 nSkipCount);

    /** Rewind the last effect of the main effect sequence by replaying all
        previous effects.
        @param nEffectCount
            The number of main sequence effects to replay.
        @param bRedisplayCurrentSlide
            When <TRUE/> then the current slide is redisplayed before the
            effects are replayed.
        @param rSlideRewindFunctor
            This functor is used to redisplay the current slide.
    */
    void asynchronousRewind (
        sal_Int32 nEffectCount,
        const bool bRedisplayCurrentSlide,
        const boost::function<void(void)>& rSlideRewindFunctor);

    /** Go to the previous slide and replay all of its main sequence effects
        (or effect groups).
        @param rPreviousSlideFunctor
            This functor is used to go to the previous slide.
    */
    void asynchronousRewindToPreviousSlide (
        const ::boost::function<void(void)>& rPreviousSlideFunctor);
};

} } // end of namespace ::slideshow::internal

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
