/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: timestamp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:25:51 $
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
        static TimeStamp always(); // is before (<) any other TimeStamp
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

