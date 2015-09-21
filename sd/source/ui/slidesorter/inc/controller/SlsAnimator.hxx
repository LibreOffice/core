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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSANIMATOR_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSANIMATOR_HXX

#include "SlideSorter.hxx"
#include "view/SlideSorterView.hxx"
#include <canvas/elapsedtime.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <sal/types.h>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <functional>
#include <vector>

namespace sd { namespace slidesorter { namespace controller {

/** Experimental class for simple eye candy animations.
*/
class Animator
    : private ::boost::noncopyable
{
public:
    /** In some circumstances we have to avoid animation and jump to the
        final animation state immediately.  Use this enum instead of a bool
        to be more expressive.
    */
    enum AnimationMode { AM_Animated, AM_Immediate };

    Animator (SlideSorter& rSlideSorter);
    ~Animator();

    /** When disposed the animator will stop its work immediately and not
        process any timer events anymore.
    */
    void Dispose();

    /** An animation object is called with values between 0 and 1 as single
        argument to its operator() method.
    */
    typedef ::std::function<void (double)> AnimationFunctor;
    typedef ::std::function<void ()> FinishFunctor;

    typedef sal_Int32 AnimationId;
    static const AnimationId NotAnAnimationId = -1;

    /** Schedule a new animation for execution.  The () operator of that
        animation will be called with increasing values between 0 and 1 for
        the specified duration.
        @param rAnimation
            The animation operation.
        @param nStartOffset
            Time in milli seconds before the animation is started.
        @param nDuration
            The duration in milli seconds.
    */
    AnimationId AddAnimation (
        const AnimationFunctor& rAnimation,
        const sal_Int32 nStartOffset,
        const sal_Int32 nDuration,
        const FinishFunctor& rFinishFunctor = FinishFunctor());

    /** Abort and remove an animation.  In order to reduce the bookkeeping
        on the caller side, it is OK to call this method with an animation
        function that is not currently being animated.  Such a call is
        silently ignored.
    */
    void RemoveAnimation (const AnimationId nAnimationId);

    /** A typical use case for this method is the temporary shutdown of the
        slidesorter when the slide sorter bar is put into a cache due to a
        change of the edit mode.
    */
    void RemoveAllAnimations();

private:
    SlideSorter& mrSlideSorter;
    Idle maIdle;
    bool mbIsDisposed;
    class Animation;
    typedef ::std::vector<boost::shared_ptr<Animation> > AnimationList;
    AnimationList maAnimations;
    ::canvas::tools::ElapsedTime maElapsedTime;

    std::unique_ptr<view::SlideSorterView::DrawLock> mpDrawLock;

    AnimationId mnNextAnimationId;

    DECL_LINK_TYPED(TimeoutHandler, Idle *, void);

    /** Execute one step of every active animation.
        @param nTime
            Time measured in milli seconds with some arbitrary reference point.
        @return
            When one or more animation has finished then <TRUE/> is
            returned.  Call CleanUpAnimationList() in this case.
    */
    bool ProcessAnimations (const double nTime);

    /** Remove animations that have expired.
    */
    void CleanUpAnimationList();

    void RequestNextFrame (const double nFrameStart = 0);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
