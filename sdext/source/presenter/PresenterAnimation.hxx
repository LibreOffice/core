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

#ifndef SDEXT_PRESENTER_ANIMATION_HXX
#define SDEXT_PRESENTER_ANIMATION_HXX

#include <sal/types.h>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace sdext { namespace presenter {

/** Base class for animations handled by a PresenterAnimator object.
    A PresenterAnimation objects basically states when it wants to be
    started, how long it runs, and in what steps it wants to be called back
    while running.
    When a PresenterAnimation object is active/running its Run() method is
    called back with increasing values between 0 and 1.
*/
class PresenterAnimation
    : private ::boost::noncopyable
{
public:
    /** Create a new PresenterAnimation object.
        @param nStartDelay
            The delay in ms (milliseconds) from this call until the new
            object is to be activated.
        @param nTotalDuration
            The duration in ms the Run() method is to be called with
            increasing values between 0 and 1.
        @param nStepDuration
            The duration between calls to Run().  This leads to approximately
            nTotalDuration/nStepDuration calls to Run().  The exact duration
            of each step may vary depending on system load an other influences.
    */
    PresenterAnimation (
        const sal_uInt64 nStartDelay,
        const sal_uInt64 nTotalDuration,
        const sal_uInt64 nStepDuration);
    virtual ~PresenterAnimation (void);

    /** Return the absolute start time in a system dependent format.
        At about this time the Run() method will be called with a value of 0.
    */
    sal_uInt64 GetStartTime (void);

    /** Return the absolute end time in a system dependent format.
        At about this time the Run() method will be called with a value of 1.
    */
    sal_uInt64 GetEndTime (void);

    /** Return the duration of each step in ms.
    */
    sal_uInt64 GetStepDuration (void);

    typedef ::boost::function<void(void)> Callback;

    /** Add a callback that is executed before Run() is called for the first
        time.
    */
    void AddStartCallback (const Callback& rCallback);

    /** Add a callback that is executed after Run() is called for the last
        time.
    */
    void AddEndCallback (const Callback& rCallback);

    /** Called with nProgress taking on values between 0 and 1.
        @param nProgress
            A value between 0 and 1.
        @param nCurrentTime
            Current time in a system dependent format.
    */
    virtual void Run (const double nProgress, const sal_uInt64 nCurrentTime) = 0;

    /** Called just before Run() is called for the first time to trigger the
        callbacks registered via the <method>AddStartCallback()</method>.
    */
    void RunStartCallbacks (void);

    /** Called just after Run() is called for the last time to trigger the
        callbacks registered via the <method>AddEndCallback()</method>.
    */
    void RunEndCallbacks (void);

private:
    const sal_uInt64 mnStartTime;
    const sal_uInt64 mnTotalDuration;
    const sal_uInt64 mnStepDuration;
    ::boost::scoped_ptr<std::vector<Callback> > mpStartCallbacks;
    ::boost::scoped_ptr<std::vector<Callback> > mpEndCallbacks;
};

sal_uInt64 GetCurrentTime (void);
inline sal_uInt32 GetSeconds (const sal_uInt64 nTime) { return sal_uInt32(nTime / 1000); }
inline sal_uInt32 GetNanoSeconds (const sal_uInt64 nTime) { return sal_uInt32((nTime % 1000) * 1000000); }

typedef ::boost::shared_ptr<PresenterAnimation> SharedPresenterAnimation;


} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
