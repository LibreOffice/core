/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsAnimator.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:33:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
