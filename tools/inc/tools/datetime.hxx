/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datetime.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:09:34 $
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
#ifndef _DATETIME_HXX
#define _DATETIME_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif

#ifndef _TIME_HXX
#include <tools/time.hxx>
#endif

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

    BOOL            IsBetween( const DateTime& rFrom,
                               const DateTime& rTo ) const;

    BOOL            IsEqualIgnore100Sec( const DateTime& rDateTime ) const
                        {
                            if ( Date::operator!=( rDateTime ) )
                                return FALSE;
                            return Time::IsEqualIgnore100Sec( rDateTime );
                        }

    BOOL            operator ==( const DateTime& rDateTime ) const
                        { return (Date::operator==( rDateTime ) &&
                                  Time::operator==( rDateTime )); }
    BOOL            operator !=( const DateTime& rDateTime ) const
                        { return (Date::operator!=( rDateTime ) ||
                                  Time::operator!=( rDateTime )); }
    BOOL            operator  >( const DateTime& rDateTime ) const;
    BOOL            operator  <( const DateTime& rDateTime ) const;
    BOOL            operator >=( const DateTime& rDateTime ) const;
    BOOL            operator <=( const DateTime& rDateTime ) const;

    long            GetSecFromDateTime( const Date& rDate ) const;
    void            MakeDateTimeFromSec( const Date& rDate, ULONG nSec );

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
