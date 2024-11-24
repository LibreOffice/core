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
#ifndef INCLUDED_TOOLS_DATETIME_HXX
#define INCLUDED_TOOLS_DATETIME_HXX

#include <tools/toolsdllapi.h>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <com/sun/star/util/DateTime.hpp>

#include <iomanip>

namespace tools
{
class Duration;
}

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC DateTime : public Date, public tools::Time
{
public:
    enum DateTimeInitSystem
    {
        SYSTEM
    };

    enum DateTimeInitEmpty
    {
        EMPTY
    };

                    explicit DateTime( DateTimeInitEmpty ) : Date( Date::EMPTY ), Time( Time::EMPTY ) {}
                    explicit DateTime( DateTimeInitSystem );
                    DateTime( const DateTime& rDateTime ) :
                        Date( rDateTime ), Time( rDateTime ) {}
                    explicit DateTime( const Date& rDate ) : Date( rDate ), Time(Time::EMPTY) {}
                    explicit DateTime( const tools::Time& rTime ) : Date(0), Time( rTime ) {}
                    DateTime( const Date& rDate, const tools::Time& rTime ) :
                        Date( rDate ), Time( rTime ) {}
                    explicit DateTime( const css::util::DateTime& rDateTime );

    css::util::DateTime
                    GetUNODateTime() const
                        { return css::util::DateTime(GetNanoSec(), GetSec(), GetMin(), GetHour(),
                              GetDay(), GetMonth(), GetYear(), false); }

    bool            IsBetween( const DateTime& rFrom,
                               const DateTime& rTo ) const;

    bool            IsEqualIgnoreNanoSec( const DateTime& rDateTime ) const
                    {
                        if ( GetDate() != rDateTime.GetDate() )
                            return false;
                        return Time::IsEqualIgnoreNanoSec( rDateTime );
                    }

    auto            operator <=>( const DateTime& rDateTime ) const
                    {
                        if (auto cmp = Date::operator<=>(rDateTime); cmp != 0)
                            return cmp;
                        return tools::Time::operator<=>(rDateTime);
                    }
    bool            operator==(const DateTime& rDateTime) const
                    {
                        return (Date::operator==(rDateTime) && tools::Time::operator==(rDateTime));
                    }

    sal_Int64       GetSecFromDateTime( const Date& rDate ) const;

    void            ConvertToUTC()       { *this -= Time::GetUTCOffset(); }
    void            ConvertToLocalTime() { *this += Time::GetUTCOffset(); }

    void            AddTime( double fTimeInDays );
    DateTime&       operator +=( const tools::Time& rTime );
    DateTime&       operator -=( const tools::Time& rTime );
    /** Duration can be negative, so adding it will subtract its value. */
    DateTime&       operator +=( const tools::Duration& rDuration );
private:
    void            NormalizeTimeRemainderAndApply( tools::Time& rTime );
public:

    TOOLS_DLLPUBLIC friend DateTime operator +( const DateTime& rDateTime, sal_Int32 nDays );
    TOOLS_DLLPUBLIC friend DateTime operator -( const DateTime& rDateTime, sal_Int32 nDays );
    TOOLS_DLLPUBLIC friend DateTime operator +( const DateTime& rDateTime, double fTimeInDays );
    TOOLS_DLLPUBLIC friend DateTime operator -( const DateTime& rDateTime, double fTimeInDays )
                        { return operator+( rDateTime, -fTimeInDays ); }
    TOOLS_DLLPUBLIC friend DateTime operator +( const DateTime& rDateTime, const tools::Time& rTime );
    TOOLS_DLLPUBLIC friend DateTime operator -( const DateTime& rDateTime, const tools::Time& rTime );
    /** Use operator-() if a duration is to be remembered or processed. */
    TOOLS_DLLPUBLIC friend tools::Duration operator -( const DateTime& rDateTime1, const DateTime& rDateTime2 );
    /** Use Sub() if the floating point "time in days" value is to be
        processed. This also takes a shortcut for whole days values (equal
        times), and only for times inflicted values uses an intermediary
        tools::Duration for conversion. Note that the resulting floating point
        value nevertheless in many cases is not an exact representation down to
        nanoseconds. */
    static  double  Sub( const DateTime& rDateTime1, const DateTime& rDateTime2 );
    TOOLS_DLLPUBLIC friend sal_Int64 operator -( const DateTime& rDateTime, const Date& rDate )
                        { return static_cast<const Date&>(rDateTime) - rDate; }
    /** Duration can be negative, so adding it will subtract its value. */
    TOOLS_DLLPUBLIC friend DateTime operator +( const DateTime& rDateTime, const tools::Duration& rDuration );

    DateTime&       operator =( const DateTime& rDateTime );
    DateTime&       operator =( const css::util::DateTime& rUDateTime );

    void            GetWin32FileDateTime( sal_uInt32 & rLower, sal_uInt32 & rUpper ) const;
    static DateTime CreateFromWin32FileDateTime( sal_uInt32 rLower, sal_uInt32 rUpper );

    /// Creates DateTime given a unix time, which is the number of seconds
    /// elapsed since Jan 1st, 1970.
    static DateTime CreateFromUnixTime( const double fSecondsSinceEpoch );
};

inline DateTime& DateTime::operator =( const DateTime& rDateTime )
{
    Date::operator=( rDateTime );
    Time::operator=( rDateTime );
    return *this;
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const DateTime& datetime)
{
    return stream << datetime.GetYear() << '-' <<
        std::setw(2) << std::setfill('0') << datetime.GetMonth() << '-' <<
        std::setw(2) << std::setfill('0') << datetime.GetDay() << ' ' <<
        std::setw(2) << std::setfill('0') << datetime.GetHour() << ':' <<
        std::setw(2) << std::setfill('0') << datetime.GetMin() << ':' <<
        std::setw(2) << std::setfill('0') << datetime.GetSec() << "." <<
        std::setw(9) << std::setfill('0') << datetime.GetNanoSec();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
