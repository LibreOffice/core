/*************************************************************************
 *
 *  $RCSfile: timestamp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:43 $
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

