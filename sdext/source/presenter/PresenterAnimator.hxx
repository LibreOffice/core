/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterAnimator.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:55:18 $
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

#ifndef SDEXT_PRESENTER_ANIMATOR_HXX
#define SDEXT_PRESENTER_ANIMATOR_HXX

#include "PresenterAnimation.hxx"
#include <cppuhelper/basemutex.hxx>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace sdext { namespace presenter {

/** Simple animation management.  Call AddAnimation to run animations
    concurrently or one of the other.  See PresenterAnimation for details of
    how to specify animations.
*/
class PresenterAnimator
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex
{
public:
    PresenterAnimator (void);
    virtual ~PresenterAnimator (void);

    /** Add an animation.  The time at which to start and end this animation
        is provided by the animation itself.
    */
    void AddAnimation (const SharedPresenterAnimation& rpAnimation);

private:
    typedef ::std::multimap<sal_uInt64,SharedPresenterAnimation> AnimationList;
    AnimationList maFutureAnimations;
    AnimationList maActiveAnimations;
    class Timer;
    ::boost::scoped_ptr<Timer> mpTimer;
    sal_uInt64 mnNextTime;

    void Process (void);
    void ActivateAnimations (const sal_uInt64 nCurrentTime);
    void ScheduleNextRun (void);
    void ScheduleNextRun (const sal_uInt64 nStartTime);

};

} } // end of namespace ::sdext::presenter

#endif
