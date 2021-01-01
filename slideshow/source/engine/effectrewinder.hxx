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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_EFFECTREWINDER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_EFFECTREWINDER_HXX

#include <animationnode.hxx>
#include <eventhandler.hxx>
#include <animationeventhandler.hxx>
#include <event.hxx>
#include <screenupdater.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <functional>
#include <memory>

namespace slideshow::internal {

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
    ~EffectRewinder();

    /** Call Dispose() before the owner of an EffectRewinder object dies so
        that the EffectRewinder can release all references to the owner.

    */
    void dispose();

    /** Store the root node of the animation tree.  It is used in
        CountMainSequenceEffects() to count the number of main sequence
        effects (or effect groups.)
    */
    void setRootAnimationNode (
        const css::uno::Reference<css::animations::XAnimationNode>& xRootNode);

    /** Store the XDrawPage to reach specific slide properties.
    */
    void setCurrentSlide (
        const css::uno::Reference<css::drawing::XDrawPage>& xSlide);

    /** Rewind one effect of the main effect sequence.  When the current
        slide has not effects or no main sequence effect has yet been played
        then switch to the previous slide and replay all of its main
        sequence effects.
        The caller has to pass two functors that redisplay the current slide
        or switch to the previous slide so that it does not have to expose
        its internals to us.  Only one of the two functors is called.
        @param rpPaintLock
            This paint lock is released after the whole asynchronous
            process of rewinding the current effect is completed.  It
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
        const ::std::shared_ptr<ScreenUpdater::UpdateLock>& rpPaintLock,
        const ::std::function<void ()>& rSlideRewindFunctor,
        const ::std::function<void ()>& rPreviousSlideFunctor);

    /** Call this method after gotoPreviousEffect() triggered a slide change
        to the previous slide.
    */
    void skipAllMainSequenceEffects();

    //FIXME: That is an opengl issue(it doesn't allow to animate some animations), remove that function when opengl fixed.
    static bool hasBlockedAnimation( const css::uno::Reference<css::animations::XAnimationNode>& xNode);

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
    css::uno::Reference<css::drawing::XDrawPage> mxCurrentSlide;
    ::std::shared_ptr<ScreenUpdater::UpdateLock> mpPaintLock;

    bool mbNonUserTriggeredMainSequenceEffectSeen;
    bool mbHasAdvancedTimeSetting; // Slide has advanced time setting or not.

    void initialize();

    bool resetEffectCount();
    /** Called by listeners when an animation (not necessarily of a main
        sequence effect) starts.
    */
    bool notifyAnimationStart (const AnimationNodeSharedPtr& rpNode);

    /** Count the number of effects (or effect groups) in the main effect
        sequence.
    */
    sal_Int32 countMainSequenceEffects();

    /** Skip the next main sequence effect.
    */
    void skipSingleMainSequenceEffects();

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
        const ::std::function<void ()>& rSlideRewindFunctor);

    /** Go to the previous slide and replay all of its main sequence effects
        (or effect groups).
        @param rPreviousSlideFunctor
            This functor is used to go to the previous slide.
    */
    void asynchronousRewindToPreviousSlide (
        const ::std::function<void ()>& rPreviousSlideFunctor);
};

} // end of namespace ::slideshow::internal

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
