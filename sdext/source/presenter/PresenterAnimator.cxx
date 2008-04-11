/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterAnimator.cxx,v $
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

#include "PresenterAnimator.hxx"

#include <osl/diagnose.h>
#include <osl/time.h>
#include <vos/timer.hxx>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace sdext { namespace presenter {



//===== PresenterAnimator::Timer ==============================================

class PresenterAnimator::Timer : public ::vos::OTimer
{
public:
    typedef ::boost::function0<void> Callback;
    Timer (const Callback& rCallback) : maCallback(rCallback){ acquire(); };
    virtual ~Timer (void) { };
    void Dispose (void) { stop(); release(); };
protected:
    virtual void SAL_CALL onShot (void) { maCallback(); };
private:
    Callback maCallback;
};




//===== PresenterAnimator =====================================================

PresenterAnimator::PresenterAnimator (void)
    : maFutureAnimations(),
      maActiveAnimations(),
      mpTimer(new Timer(::boost::bind(&PresenterAnimator::Process, this))),
      mnNextTime(0)
{
}




PresenterAnimator::~PresenterAnimator (void)
{
    mpTimer->Dispose();
}





void PresenterAnimator::AddAnimation (const SharedPresenterAnimation& rpAnimation)
{
    ::osl::MutexGuard aGuard (m_aMutex);

    maFutureAnimations.insert(AnimationList::value_type(rpAnimation->GetStartTime(), rpAnimation));
    ScheduleNextRun();
}




void PresenterAnimator::Process (void)
{
    ::osl::MutexGuard aGuard (m_aMutex);

    mnNextTime = 0;
    mpTimer->stop();

    const sal_uInt64 nCurrentTime (GetCurrentTime());

    ActivateAnimations(nCurrentTime);

    while ( ! maActiveAnimations.empty())
    {
        sal_uInt64 nRequestedTime (maActiveAnimations.begin()->first);
        SharedPresenterAnimation pAnimation (maActiveAnimations.begin()->second);

        if (nRequestedTime > nCurrentTime)
            break;

        maActiveAnimations.erase(maActiveAnimations.begin());

        const double nTotalDuration (double(pAnimation->GetEndTime() - pAnimation->GetStartTime()));
        double nProgress (nTotalDuration > 0 ? (nCurrentTime - pAnimation->GetStartTime()) / nTotalDuration : 1);
        if (nProgress <= 0)
            nProgress = 0;
        else if (nProgress >= 1)
            nProgress = 1;

        OSL_TRACE("running animation step at %d (requested was %d)\n", nCurrentTime, nRequestedTime);
        pAnimation->Run(nProgress, nCurrentTime);

        if (nCurrentTime < pAnimation->GetEndTime())
            maActiveAnimations.insert(
                AnimationList::value_type(
                    nCurrentTime + pAnimation->GetStepDuration(),
                    pAnimation));
        else
            pAnimation->RunEndCallbacks();
    }

    ScheduleNextRun();
}




void PresenterAnimator::ActivateAnimations (const sal_uInt64 nCurrentTime)
{
    while ( ! maFutureAnimations.empty()
        && maFutureAnimations.begin()->first <= nCurrentTime)
    {
        SharedPresenterAnimation pAnimation (maFutureAnimations.begin()->second);
        maActiveAnimations.insert(*maFutureAnimations.begin());
        maFutureAnimations.erase(maFutureAnimations.begin());
        pAnimation->RunStartCallbacks();
    }
}




void PresenterAnimator::ScheduleNextRun (void)
{
    sal_uInt64 nStartTime (0);

    if ( ! maActiveAnimations.empty())
    {
        nStartTime  = maActiveAnimations.begin()->first;
        if ( ! maFutureAnimations.empty())
            if (maFutureAnimations.begin()->first < nStartTime)
                nStartTime = maFutureAnimations.begin()->first;
    }
    else if ( ! maFutureAnimations.empty())
        nStartTime = maFutureAnimations.begin()->first;

    if (nStartTime > 0)
        ScheduleNextRun(nStartTime);
}




void PresenterAnimator::ScheduleNextRun (const sal_uInt64 nStartTime)
{
    if (mnNextTime==0 || nStartTime<mnNextTime)
    {
        mnNextTime = nStartTime;
        mpTimer->stop();
        ::vos::TTimeValue aTimeValue (GetSeconds(mnNextTime), GetNanoSeconds(mnNextTime));
        mpTimer->setAbsoluteTime(aTimeValue);
        mpTimer->start();
    }
}

} } // end of namespace ::sdext::presenter
