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

class TOOLS_DLLPUBLIC SAL_WARN_UNUSED DateTime : public Date, public tools::Time
{
public:
    enum DateTimeInitSystem
    {
        SYSTEM
    };

    // TODO temporary until all uses are inspected and resolved
    enum DateTimeInitEmpty
    {
        EMPTY
    };

                    DateTime( DateTimeInitEmpty ) : Date( Date::EMPTY ), Time( Time::EMPTY ) {}
                    DateTime( DateTimeInitSystem ) : Date( Date::SYSTEM ), Time( Time::SYSTEM ) {}
                    DateTime( const DateTime& rDateTime ) :
                        Date( rDateTime ), Time( rDateTime ) {}
                    DateTime( const Date& rDate ) : Date( rDate ), Time(0) {}
                    DateTime( const tools::Time& rTime ) : Date(0), Time( rTime ) {}
                    DateTime( const Date& rDate, const tools::Time& rTime ) :
                        Date( rDate ), Time( rTime ) {}
                    DateTime( const css::util::DateTime& rDateTime );

    css::util::DateTime
                    GetUNODateTime() const
                        { return css::util::DateTime(GetNanoSec(), GetSec(), GetMin(), GetHour(),
                              GetDay(), GetMonth(), GetYear(), false); }

    bool            IsBetween( const DateTime& rFrom,
                               const DateTime& rTo ) const;

    bool            IsEqualIgnoreNanoSec( const DateTime& rDateTime ) const
                    {
                        if ( Date::operator!=( rDateTime ) )
                            return false;
                        return Time::IsEqualIgnoreNanoSec( rDateTime );
                    }

    bool            operator ==( const DateTime& rDateTime ) const
                        { return (Date::operator==( rDateTime ) &&
                                  Time::operator==( rDateTime )); }
    bool            operator !=( const DateTime& rDateTime ) const
                        { return (Date::operator!=( rDateTime ) ||
                                  Time::operator!=( rDateTime )); }
    bool            operator  >( const DateTime& rDateTime ) const;
    bool            operator  <( const DateTime& rDateTime ) const;
    bool            operator >=( const DateTime& rDateTime ) const;
    bool            operator <=( const DateTime& rDateTime ) const;

    long            GetSecFromDateTime( const Date& rDate ) const;

    void            ConvertToUTC()       { *this -= Time::GetUTCOffset(); }
    void            ConvertToLocalTime() { *this += Time::GetUTCOffset(); }

    DateTime&       operator +=( long nDays )
                        { Date::operator+=( nDays ); return *this; }
    DateTime&       operator -=( long nDays )
                        { Date::operator-=( nDays ); return *this; }
    DateTime&       operator +=( double fTimeInDays );
    DateTime&       operator -=( double fTimeInDays )
                        { return operator+=( -fTimeInDays ); }
    DateTime&       operator +=( const tools::Time& rTime );
    DateTime&       operator -=( const tools::Time& rTime );

    TOOLS_DLLPUBLIC friend DateTime operator +( const DateTime& rDateTime, long nDays );
    TOOLS_DLLPUBLIC friend DateTime operator -( const DateTime& rDateTime, long nDays );
    TOOLS_DLLPUBLIC friend DateTime operator +( const DateTime& rDateTime, double fTimeInDays );
    TOOLS_DLLPUBLIC friend DateTime operator -( const DateTime& rDateTime, double fTimeInDays )
                        { return operator+( rDateTime, -fTimeInDays ); }
    TOOLS_DLLPUBLIC friend DateTime operator +( const DateTime& rDateTime, const tools::Time& rTime );
    TOOLS_DLLPUBLIC friend DateTime operator -( const DateTime& rDateTime, const tools::Time& rTime );
    TOOLS_DLLPUBLIC friend double   operator -( const DateTime& rDateTime1, const DateTime& rDateTime2 );
    TOOLS_DLLPUBLIC friend long     operator -( const DateTime& rDateTime, const Date& rDate )
                        { return (const Date&) rDateTime - rDate; }

    DateTime&       operator =( const DateTime& rDateTime );

    void            GetWin32FileDateTime( sal_uInt32 & rLower, sal_uInt32 & rUpper );
    static DateTime CreateFromWin32FileDateTime( const sal_uInt32 & rLower, const sal_uInt32 & rUpper );
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
