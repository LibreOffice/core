/*************************************************************************
 *
 *  $RCSfile: timer.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _VOS_TIMER_HXX_
#define _VOS_TIMER_HXX_

#ifndef _VOS_REFERNCE_HXX_
#   include <vos/refernce.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#   include <vos/mutex.hxx>
#endif
#ifndef _OSL_TIME_H_
#   include <osl/time.h>
#endif


#ifdef _USE_NAMESPACE
namespace vos
{
#endif


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


#ifdef _USE_NAMESPACE
}
#endif


#endif  //_VOS_TIMER_HXX_


