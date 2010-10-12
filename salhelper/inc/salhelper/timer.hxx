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


#ifndef _SALHELPER_TIMER_HXX_
#define _SALHELPER_TIMER_HXX_

#include <salhelper/simplereferenceobject.hxx>
#include <osl/time.h>

namespace salhelper
{

/** Helper class for easier manipulation with TimeValue.
 *
 *  Times are seconds in UTC since 01.01.1970
 */
struct TTimeValue : public TimeValue
{
    TTimeValue()
    {
        Seconds = 0;
        Nanosec = 0;
    }

    TTimeValue( sal_uInt32 Secs, sal_uInt32 Nano )
    {
        Seconds = Secs;
        Nanosec = Nano;

        normalize();
    }

    TTimeValue(sal_uInt32 MilliSecs)
    {
        Seconds = MilliSecs / 1000L;
        Nanosec = (MilliSecs % 1000) * 1000000L;

        normalize();
    }

    TTimeValue( const TTimeValue& rTimeValue )
    {
        Seconds = rTimeValue.Seconds;
        Nanosec = rTimeValue.Nanosec;

        normalize();
    }

    TTimeValue( const TimeValue& rTimeValue )
    {
        Seconds = rTimeValue.Seconds;
        Nanosec = rTimeValue.Nanosec;

        normalize();
    }

    void SAL_CALL normalize()
    {
        if ( Nanosec > 1000000000 )
        {
            Seconds += Nanosec / 1000000000;
            Nanosec %= 1000000000;
        }
    }

    void SAL_CALL addTime( const TTimeValue& Delta )
    {
        Seconds += Delta.Seconds;
        Nanosec += Delta.Nanosec;

        normalize();
    }

    sal_Bool SAL_CALL isEmpty() const
    {
        return ( ( Seconds == 0 ) && ( Nanosec == 0 ) );
    }
};

inline sal_Bool operator<( const TTimeValue& rTimeA, const TTimeValue& rTimeB )
{
    if ( rTimeA.Seconds < rTimeB.Seconds )
        return sal_True;
    else if ( rTimeA.Seconds > rTimeB.Seconds )
        return sal_False;
    else
        return ( rTimeA.Nanosec < rTimeB.Nanosec );
}

inline sal_Bool operator>( const TTimeValue& rTimeA, const TTimeValue& rTimeB )
{
    if ( rTimeA.Seconds > rTimeB.Seconds )
        return sal_True;
    else if ( rTimeA.Seconds < rTimeB.Seconds )
        return sal_False;
    else
        return ( rTimeA.Nanosec > rTimeB.Nanosec );
}

inline sal_Bool operator==( const TTimeValue& rTimeA, const TTimeValue& rTimeB )
{
    return ( ( rTimeA.Seconds == rTimeB.Seconds ) &&
             ( rTimeA.Nanosec == rTimeB.Nanosec ) );
}

class TimerManager;

/** Interface for the Timer and handling the event
*/
class Timer : public salhelper::SimpleReferenceObject
{
public:

    /** Constructor.
     */
    Timer();

    /** Constructor.
     */
    Timer( const TTimeValue& Time );

    /** Constructor.
     */
    Timer( const TTimeValue& Time, const TTimeValue& RepeatTime );

    /** Start timer.
     */
    void        SAL_CALL start();

    /** Abort timer prematurely.
     */
    void        SAL_CALL stop();

    /** Returns sal_True if timer is running.
     */
    sal_Bool    SAL_CALL isTicking() const;

    /** Is the timer expired?
     */
    sal_Bool    SAL_CALL isExpired() const;

    /** Does pTimer expires before us?
     */
    sal_Bool    SAL_CALL expiresBefore( const Timer* pTimer ) const;

    /** Set the absolute time when the timer should fire.
     */
    void        SAL_CALL setAbsoluteTime( const TTimeValue& Time );

    /** Set the time to fire to 'now' + Remaining.
     */
    void        SAL_CALL setRemainingTime( const TTimeValue& Remaining );

    /** Set the time to fire to 'now' + Remaining with repeat interveal
     * Repeat.
     */
    void        SAL_CALL setRemainingTime( const TTimeValue& Remaining, const TTimeValue& Repeat );

    /** Adds Time to the 'fire time'.
     */
    void        SAL_CALL addTime( const TTimeValue& Time );

    /** Returns the remaining time before timer expiration relative to now.
     */
    TTimeValue  SAL_CALL getRemainingTime() const;

protected:

    /** Destructor.
     */
    virtual ~Timer();

    /** What should be done when the 'timer fires'.
     */
    virtual void SAL_CALL onShot() = 0;

protected:

    /** holds (initial) exparation time of this timer.
     */
    TTimeValue  m_aTimeOut;

    /** holds the time of exparation of this timer.
     */
    TTimeValue  m_aExpired;

    /** holds the time interveal of successive expirations.
     */
    TTimeValue  m_aRepeatDelta;

    /** Pointer to the next timer (to fire).
     */
    Timer*      m_pNext;

private:

    /** Copy constructor disabled.
     */
    Timer( const Timer& rTimer );

    /** Assignment operator disabled.
     */
    void SAL_CALL operator=( const Timer& rTimer );

    friend class TimerManager;
};

}

#endif  //_SALHELPER_TIMER_HXX_
