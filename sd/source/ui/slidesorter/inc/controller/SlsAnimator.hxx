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

#ifndef SD_SLIDESORTER_CONTROLLER_ANIMATOR_HXX
#define SD_SLIDESORTER_CONTROLLER_ANIMATOR_HXX

#include "SlideSorter.hxx"
#include "view/SlideSorterView.hxx"
#include <canvas/elapsedtime.hxx>
#include <vcl/timer.hxx>
#include <sal/types.h>
#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>


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
    ~Animator (void);

    /** When disposed the animator will stop its work immediately and not
        process any timer events anymore.
    */
    void Dispose (void);

    /** An animation object is called with values between 0 and 1 as single
        argument to its operator() method.
    */
    typedef ::boost::function1<void, double> AnimationFunctor;
    typedef ::boost::function0<void> FinishFunctor;

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

    AnimationId AddInfiniteAnimation (
        const AnimationFunctor& rAnimation,
        const double nDelta);

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
    void RemoveAllAnimations (void);

private:
    SlideSorter& mrSlideSorter;
    Timer maTimer;
    bool mbIsDisposed;
    class Animation;
    typedef ::std::vector<boost::shared_ptr<Animation> > AnimationList;
    AnimationList maAnimations;
    ::canvas::tools::ElapsedTime maElapsedTime;

    ::boost::scoped_ptr<view::SlideSorterView::DrawLock> mpDrawLock;

    AnimationId mnNextAnimationId;

    DECL_LINK(TimeoutHandler, Timer*);

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
    void CleanUpAnimationList (void);

    void RequestNextFrame (const double nFrameStart = 0);
};


} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
