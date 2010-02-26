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

#ifndef CONFIGMGR_TIMESTAMP_HXX
#define CONFIGMGR_TIMESTAMP_HXX

#include <vos/timer.hxx>

namespace configmgr
{
////////////////////////////////////////////////////////////////////////////////

    class TimeInterval
    {
        vos::TTimeValue m_aTime;
    public:
        TimeInterval() : m_aTime()
        {}

        explicit
        TimeInterval(sal_uInt32 nSeconds) : m_aTime(nSeconds,0)
        {}

        explicit
        TimeInterval(const TimeValue& rTimeValue) : m_aTime(rTimeValue)
        {}

        sal_Bool isEmpty() const { return m_aTime.isEmpty(); }

        vos::TTimeValue const& getTimeValue() const { return m_aTime; }
    };
////////////////////////////////////////////////////////////////////////////////

    class TimeStamp
    {
        vos::TTimeValue m_aTime;
    public:
        TimeStamp() : m_aTime()
        {}

        explicit
        TimeStamp(TimeValue const& rTimeValue) : m_aTime(rTimeValue)
        {}

        TimeStamp& operator += (TimeInterval const& aInterval)
        { m_aTime.addTime(aInterval.getTimeValue()); return *this; }

        vos::TTimeValue const& getTimeValue() const { return m_aTime; }

        sal_Bool isNever() const;

        static TimeStamp getCurrentTime();
        static TimeStamp never(); // is later than (>) any other TimeStamp
    };

    inline
    TimeStamp operator +(TimeStamp const& aTime, TimeInterval const& aInterval)
    {
        TimeStamp aResult(aTime);
        aResult += aInterval;
        return aResult;
    }
    inline
    TimeStamp operator +(TimeInterval const& aInterval, TimeStamp const& aTime)
    {
        TimeStamp aResult(aTime);
        aResult += aInterval;
        return aResult;
    }
////////////////////////////////////////////////////////////////////////////////
    inline sal_Bool operator ==(TimeStamp const& lhs, TimeStamp const& rhs)
    { return lhs.getTimeValue() == rhs.getTimeValue(); }
    inline sal_Bool operator < (TimeStamp const& lhs, TimeStamp const& rhs)
    { return lhs.getTimeValue() < rhs.getTimeValue(); }
    inline sal_Bool operator > (TimeStamp const& lhs, TimeStamp const& rhs)
    { return lhs.getTimeValue() > rhs.getTimeValue(); }

    inline sal_Bool operator !=(TimeStamp const& lhs, TimeStamp const& rhs)
    { return !(lhs == rhs); }
    inline sal_Bool operator <=(TimeStamp const& lhs, TimeStamp const& rhs)
    { return !(rhs < lhs); }
    inline sal_Bool operator >=(TimeStamp const& lhs, TimeStamp const& rhs)
    { return !(lhs < rhs); }

    inline sal_Bool TimeStamp::isNever() const
    {
        return never() <= *this;
    }
////////////////////////////////////////////////////////////////////////////////

    struct ltTimeStamp //: std::binary_function<TimeStamp,TimeStamp,bool>
    {
        bool operator()(TimeStamp const& lhs, TimeStamp const& rhs) const
        { return !!(lhs < rhs); }
    };

////////////////////////////////////////////////////////////////////////////////
} // namespace configmgr

#endif // CONFIGMGR_TIMESTAMP_HXX

