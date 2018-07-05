/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#ifndef INCLUDED_SALHELPER_TIMER_HXX
#define INCLUDED_SALHELPER_TIMER_HXX

#include "salhelper/simplereferenceobject.hxx"
#include "osl/time.h"
#include "salhelper/salhelperdllapi.h"

namespace salhelper
{

/** Helper class for easier manipulation with TimeValue.
 *
 *  Times are seconds in UTC since 01.01.1970
 */
struct SAL_WARN_UNUSED TTimeValue : public TimeValue
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

#if 0
    TTimeValue( const TTimeValue& rTimeValue ):
        TimeValue(rTimeValue)
    {
        normalize();
    }
#endif

    TTimeValue( const TimeValue& rTimeValue )
    {
        Seconds = rTimeValue.Seconds;
        Nanosec = rTimeValue.Nanosec;

        normalize();
    }

#if 0
    TTimeValue & operator =(TTimeValue const & other) {
        Seconds = other.Seconds;
        Nanosec = other.Nanosec;
        return *this;
    }
#endif

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

    bool SAL_CALL isEmpty() const
    {
        return ( ( Seconds == 0 ) && ( Nanosec == 0 ) );
    }
};

inline bool operator<( const TTimeValue& rTimeA, const TTimeValue& rTimeB )
{
    if ( rTimeA.Seconds < rTimeB.Seconds )
        return true;
    else if ( rTimeA.Seconds > rTimeB.Seconds )
        return false;
    else
        return ( rTimeA.Nanosec < rTimeB.Nanosec );
}

inline bool operator>( const TTimeValue& rTimeA, const TTimeValue& rTimeB )
{
    if ( rTimeA.Seconds > rTimeB.Seconds )
        return true;
    else if ( rTimeA.Seconds < rTimeB.Seconds )
        return false;
    else
        return ( rTimeA.Nanosec > rTimeB.Nanosec );
}

inline bool operator==( const TTimeValue& rTimeA, const TTimeValue& rTimeB )
{
    return ( ( rTimeA.Seconds == rTimeB.Seconds ) &&
             ( rTimeA.Nanosec == rTimeB.Nanosec ) );
}

class TimerManager;

/** Interface for the Timer and handling the event
*/
class SALHELPER_DLLPUBLIC Timer : public salhelper::SimpleReferenceObject
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
    virtual ~Timer() SAL_OVERRIDE;

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

    /** Copy constructor deleted.
     */
    Timer( const Timer& rTimer ) SAL_DELETED_FUNCTION;

    /** Copy assignment operator deleted.
     */
    void SAL_CALL operator=( const Timer& rTimer ) SAL_DELETED_FUNCTION;

    friend class TimerManager;
};

}

#endif // INCLUDED_SALHELPER_TIMER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
