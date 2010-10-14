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


#ifndef _VOS_TIMER_HXX_
#define _VOS_TIMER_HXX_

#   include <vos/refernce.hxx>
#   include <vos/mutex.hxx>
#   include <osl/time.h>


namespace vos
{

/////////////////////////////////////////////////////////////////////////////
//
// TTimeValue
//

/** <code> struct TTimeValue </code> : class for times. Times are seconds in UTC since 01.01.1970
 */
struct TTimeValue : public TimeValue
{
    TTimeValue()
    { Seconds = 0; Nanosec = 0; }

    TTimeValue(sal_uInt32 Seconds, sal_uInt32 Nano);

    TTimeValue(sal_uInt32 MilliSecs)
    { Seconds = MilliSecs / 1000L; Nanosec = (MilliSecs % 1000) * 1000000L; }

    TTimeValue(const TTimeValue& rTimeValue)
    { Seconds = rTimeValue.Seconds; Nanosec = rTimeValue.Nanosec; }

    TTimeValue(const TimeValue& rTimeValue)
    { Seconds = rTimeValue.Seconds; Nanosec = rTimeValue.Nanosec; }

    void        SAL_CALL normalize();

    void        SAL_CALL addTime(const TTimeValue& Delta);

    sal_Bool    SAL_CALL isEmpty() const;
};

inline void TTimeValue::normalize()
{
    if (Nanosec > 1000000000)
    {
        Seconds += Nanosec / 1000000000;
        Nanosec %= 1000000000;
    }
}

inline TTimeValue::TTimeValue(sal_uInt32 Secs, sal_uInt32 Nano)
{
    Seconds = Secs;
    Nanosec = Nano;

    normalize();
}

inline void TTimeValue::addTime(const TTimeValue& Time)
{
    Seconds += Time.Seconds;
    Nanosec += Time.Nanosec;

    normalize();
}

inline sal_Bool TTimeValue::isEmpty() const
{
    return ((Seconds == 0) && (Nanosec == 0));
}

inline sal_Bool operator<(const TTimeValue& rTimeA, const TTimeValue& rTimeB)
{
    if (rTimeA.Seconds < rTimeB.Seconds)
        return sal_True;
    else if (rTimeA.Seconds > rTimeB.Seconds)
        return sal_False;
    else
        return (rTimeA.Nanosec < rTimeB.Nanosec);
}

inline sal_Bool operator>(const TTimeValue& rTimeA, const TTimeValue& rTimeB)
{
    if (rTimeA.Seconds > rTimeB.Seconds)
        return sal_True;
    else if (rTimeA.Seconds < rTimeB.Seconds)
        return sal_False;
    else
        return (rTimeA.Nanosec > rTimeB.Nanosec);
}

inline sal_Bool operator==(const TTimeValue& rTimeA, const TTimeValue& rTimeB)
{
    return ((rTimeA.Seconds == rTimeB.Seconds) &&
            (rTimeA.Nanosec == rTimeB.Nanosec));
}


/////////////////////////////////////////////////////////////////////////////
//
//  Timer class
//

class OTimerManager;

/** <code> class OTimer </code> : Interface for the Timer and handling the event
*/
class OTimer : virtual public OReference , virtual public OObject
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OTimer, vos));

public:

    /// constructor
      OTimer();
    /// constructor
      OTimer(const TTimeValue& Time);
    /// constructor
      OTimer(const TTimeValue& Time, const TTimeValue& RepeatTime);
      /// start timer.
      void SAL_CALL start();
      /// abort timer prematurely.
      void SAL_CALL stop();
      /// returns <code> sal_True </code> if timer is running.
      sal_Bool  SAL_CALL isTicking() const;
    /// is the timer expired?
      sal_Bool  SAL_CALL isExpired() const;
    /// does <code> pTimer </code> expires before us?
      sal_Bool    SAL_CALL expiresBefore(const OTimer* pTimer) const;
    /// set the absolute time when the timer should fire
      void      SAL_CALL setAbsoluteTime(const TTimeValue& Time);
    /// set the time to fire to 'now' + <code> Remaining </code>
      void      SAL_CALL setRemainingTime(const TTimeValue& Remaining);
    /// set the time to fire to 'now' + <code> Remaining </code> with repeat interveal <code> Repeat </code>
      void      SAL_CALL setRemainingTime(const TTimeValue& Remaining, const TTimeValue& Repeat);
    /// adds <code> Time </code> to the 'fire time'
      void      SAL_CALL addTime(const TTimeValue& Time);
    /// returns the remaining time before timer expiration relative to now
      TTimeValue    SAL_CALL getRemainingTime() const;

protected:

    /// destructor
    virtual ~OTimer();
    /// what should be done when the 'timer fires'
    virtual void SAL_CALL onShot() = 0;

    /// holds (initial) exparation time of this timer
    TTimeValue  m_TimeOut;
    /// holds the time of exparation of this timer
    TTimeValue  m_Expired;
    /// holds the time interveal of successive exparations
    TTimeValue  m_RepeatDelta;
    /// Pointer to the next timer (to fire)
      OTimer*       m_pNext;

private:

    /// copy constructor disabled
    OTimer(const OTimer& rTimer);
    /// assignment operator disabled
    void SAL_CALL operator=(const OTimer& rTimer);

    friend class OTimerManager;
};

}


#endif  //_VOS_TIMER_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
