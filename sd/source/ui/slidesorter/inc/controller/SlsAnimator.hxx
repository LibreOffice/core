/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsAnimator.hxx,v $
 *
 * $Revision: 1.3 $
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
    Animator (SlideSorter& rSlideSorter);
    ~Animator (void);

    /** An animation object is called with values between 0 and 1 as single
        argument to its operator() method.
    */
    typedef ::boost::function1<void, double> AnimationFunction;

    /** Schedule a new animation for execution.  The () operator of that
        animation will be called with increasing values between 0 and 1 for
        the specified duration.
        @param rAnimation
            The animation operation.
        @param nDuration
            The duration in milli seconds.
    */
    void AddAnimation (
        const AnimationFunction& rAnimation,
        const sal_Int32 nDuration);

private:
    SlideSorter& mrSlideSorter;
    Timer maTimer;

    class Animation;
    typedef ::std::vector<boost::shared_ptr<Animation> > AnimationList;
    AnimationList maAnimations;

    class DrawLock;
    ::boost::scoped_ptr<DrawLock> mpDrawLock;

    DECL_LINK(TimeoutHandler, Timer*);

    /** Execute one step of every active animation.
        @return
            When one or more animation has finished then <TRUE/> is
            returned.  Call CleanUpAnimationList() in this case.
    */
    bool ServeAnimations (void);

    /** Remove animations that have expired.
    */
    void CleanUpAnimationList (void);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif
