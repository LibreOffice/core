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
    sal_Int32 mnCurrentTaskId;
    sal_uInt64 mnNextTime;

    void Process (void);
    void ActivateAnimations (const sal_uInt64 nCurrentTime);
    void ScheduleNextRun (void);
    void ScheduleNextRun (const sal_uInt64 nStartTime);

};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
