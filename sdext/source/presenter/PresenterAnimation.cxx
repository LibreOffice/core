/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterAnimation.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:54:40 $
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

#include "PresenterAnimation.hxx"

#include <osl/time.h>

namespace sdext { namespace presenter {

sal_uInt64 GetCurrentTime (void)
{
    TimeValue aTimeValue;
    if (osl_getSystemTime(&aTimeValue))
        return sal_uInt64(aTimeValue.Seconds * 1000.0 + aTimeValue.Nanosec / 1000000.0);
    else
        return 0;
}




PresenterAnimation::PresenterAnimation (
    const sal_uInt64 nStartDelay,
    const sal_uInt64 nTotalDuration,
    const sal_uInt64 nStepDuration)
    : mnStartTime(GetCurrentTime()+nStartDelay),
      mnTotalDuration(nTotalDuration),
      mnStepDuration(nStepDuration),
      mpStartCallbacks(),
      mpEndCallbacks()
{
}




PresenterAnimation::~PresenterAnimation (void)
{
}




sal_uInt64 PresenterAnimation::GetStartTime (void)
{
    return mnStartTime;
}




sal_uInt64 PresenterAnimation::GetEndTime (void)
{
    return mnStartTime + mnTotalDuration;
}




sal_uInt64 PresenterAnimation::GetStepDuration (void)
{
    return mnStepDuration;
}




void PresenterAnimation::AddStartCallback (const Callback& rCallback)
{
    if (mpStartCallbacks.get() == NULL)
        mpStartCallbacks.reset(new ::std::vector<Callback>());
    mpStartCallbacks->push_back(rCallback);
}




void PresenterAnimation::AddEndCallback (const Callback& rCallback)
{
    if (mpEndCallbacks.get() == NULL)
        mpEndCallbacks.reset(new ::std::vector<Callback>());
    mpEndCallbacks->push_back(rCallback);
}



void PresenterAnimation::RunStartCallbacks (void)
{
    if (mpStartCallbacks.get() != NULL)
    {
        ::std::vector<Callback>::const_iterator iCallback;
        for (iCallback=mpStartCallbacks->begin(); iCallback!=mpStartCallbacks->end(); ++iCallback)
            (*iCallback)();
    }
}




void PresenterAnimation::RunEndCallbacks (void)
{
    if (mpEndCallbacks.get() != NULL)
    {
        ::std::vector<Callback>::const_iterator iCallback;
        for (iCallback=mpEndCallbacks->begin(); iCallback!=mpEndCallbacks->end(); ++iCallback)
            (*iCallback)();
    }
}




} } // end of namespace ::sdext::presenter
