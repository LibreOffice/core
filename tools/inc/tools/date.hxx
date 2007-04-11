/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: date.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:09:24 $
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
#ifndef _DATE_HXX
#define _DATE_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class ResId;

// --------------
// - Date-Types -
// --------------

enum DayOfWeek { MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY,
                 SATURDAY, SUNDAY };

// --------
// - Date -
// --------

class TOOLS_DLLPUBLIC Date
{
private:
    sal_uInt32      nDate;

public:
                    Date();
                    Date( const ResId & rResId );
                    Date( sal_uInt32 _nDate ) { Date::nDate = _nDate; }
                    Date( const Date& rDate )
                        { nDate = rDate.nDate; }
                    Date( USHORT nDay, USHORT nMonth, USHORT nYear )
                        { nDate = (   sal_uInt32( nDay   % 100 ) ) +
                                  ( ( sal_uInt32( nMonth % 100 ) ) * 100 ) +
                                  ( ( sal_uInt32( nYear  % 10000 ) ) * 10000); }

    void            SetDate( sal_uInt32 nNewDate ) { nDate = nNewDate; }
    sal_uInt32      GetDate() const { return nDate; }

    void            SetDay( USHORT nNewDay );
    void            SetMonth( USHORT nNewMonth );
    void            SetYear( USHORT nNewYear );
    USHORT          GetDay() const { return (USHORT)(nDate % 100); }
    USHORT          GetMonth() const { return (USHORT)((nDate / 100) % 100); }
    USHORT          GetYear() const { return (USHORT)(nDate / 10000); }

    DayOfWeek       GetDayOfWeek() const;
    USHORT          GetDayOfYear() const;
    /** nMinimumNumberOfDaysInWeek: how many days of a week must reside in the
        first week of a year. */
    USHORT          GetWeekOfYear( DayOfWeek eStartDay = MONDAY,
                                   sal_Int16 nMinimumNumberOfDaysInWeek = 4 ) const;

    USHORT          GetDaysInMonth() const;
    USHORT          GetDaysInYear() const { return (IsLeapYear()) ? 366 : 365; }
    BOOL            IsLeapYear() const;
    BOOL            IsValid() const;

    BOOL            IsBetween( const Date& rFrom, const Date& rTo ) const
                        { return ((nDate >= rFrom.nDate) &&
                                 (nDate <= rTo.nDate)); }

    BOOL            operator ==( const Date& rDate ) const
                        { return (nDate == rDate.nDate); }
    BOOL            operator !=( const Date& rDate ) const
                        { return (nDate != rDate.nDate); }
    BOOL            operator  >( const Date& rDate ) const
                        { return (nDate > rDate.nDate); }
    BOOL            operator  <( const Date& rDate ) const
                        { return (nDate < rDate.nDate); }
    BOOL            operator >=( const Date& rDate ) const
                        { return (nDate >= rDate.nDate); }
    BOOL            operator <=( const Date& rDate ) const
                        { return (nDate <= rDate.nDate); }

    Date&           operator =( const Date& rDate )
                        { nDate = rDate.nDate; return *this; }
    Date&           operator +=( long nDays );
    Date&           operator -=( long nDays );
    Date&           operator ++();
    Date&           operator --();
#ifndef MPW33
    Date            operator ++( int );
    Date            operator --( int );
#endif

    TOOLS_DLLPUBLIC friend Date     operator +( const Date& rDate, long nDays );
    TOOLS_DLLPUBLIC friend Date     operator -( const Date& rDate, long nDays );
    TOOLS_DLLPUBLIC friend long     operator -( const Date& rDate1, const Date& rDate2 );
};

#endif // _DATE_HXX
