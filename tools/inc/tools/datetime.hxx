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
#ifndef _DATETIME_HXX
#define _DATETIME_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>
#include <tools/date.hxx>
#include <tools/time.hxx>

// ------------
// - DateTime -
// ------------

class TOOLS_DLLPUBLIC DateTime : public Date, public Time
{
public:
                    DateTime() : Date(), Time() {}
                    DateTime( const DateTime& rDateTime ) :
                        Date( rDateTime ), Time( rDateTime ) {}
                    DateTime( const Date& rDate ) : Date( rDate ), Time(0) {}
                    DateTime( const Time& rTime ) : Date(0), Time( rTime ) {}
                    DateTime( const Date& rDate, const Time& rTime ) :
                        Date( rDate ), Time( rTime ) {}

    sal_Bool            IsBetween( const DateTime& rFrom,
                               const DateTime& rTo ) const;

    sal_Bool            IsEqualIgnore100Sec( const DateTime& rDateTime ) const
                        {
                            if ( Date::operator!=( rDateTime ) )
                                return sal_False;
                            return Time::IsEqualIgnore100Sec( rDateTime );
                        }

    sal_Bool            operator ==( const DateTime& rDateTime ) const
                        { return (Date::operator==( rDateTime ) &&
                                  Time::operator==( rDateTime )); }
    sal_Bool            operator !=( const DateTime& rDateTime ) const
                        { return (Date::operator!=( rDateTime ) ||
                                  Time::operator!=( rDateTime )); }
    sal_Bool            operator  >( const DateTime& rDateTime ) const;
    sal_Bool            operator  <( const DateTime& rDateTime ) const;
    sal_Bool            operator >=( const DateTime& rDateTime ) const;
    sal_Bool            operator <=( const DateTime& rDateTime ) const;

    long            GetSecFromDateTime( const Date& rDate ) const;
    void            MakeDateTimeFromSec( const Date& rDate, sal_uIntPtr nSec );

    void            ConvertToUTC()       { *this -= Time::GetUTCOffset(); }
    void            ConvertToLocalTime() { *this += Time::GetUTCOffset(); }

    DateTime&       operator +=( long nDays )
                        { Date::operator+=( nDays ); return *this; }
    DateTime&       operator -=( long nDays )
                        { Date::operator-=( nDays ); return *this; }
    DateTime&       operator +=( double fTimeInDays );
    DateTime&       operator -=( double fTimeInDays )
                        { return operator+=( -fTimeInDays ); }
    DateTime&       operator +=( const Time& rTime );
    DateTime&       operator -=( const Time& rTime );

    TOOLS_DLLPUBLIC friend DateTime operator +( const DateTime& rDateTime, long nDays );
    TOOLS_DLLPUBLIC friend DateTime operator -( const DateTime& rDateTime, long nDays );
    TOOLS_DLLPUBLIC friend DateTime operator +( const DateTime& rDateTime, double fTimeInDays );
    TOOLS_DLLPUBLIC friend DateTime operator -( const DateTime& rDateTime, double fTimeInDays )
                        { return operator+( rDateTime, -fTimeInDays ); }
    TOOLS_DLLPUBLIC friend DateTime operator +( const DateTime& rDateTime, const Time& rTime );
    TOOLS_DLLPUBLIC friend DateTime operator -( const DateTime& rDateTime, const Time& rTime );
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

#endif // _DATETIME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
