/*************************************************************************
 *
 *  $RCSfile: datetime.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2000-11-10 17:54:10 $
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

#include <datetime.hxx>
#include <solmath.hxx>

/*************************************************************************
|*
|*    DateTime::IsBetween()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 18.05.92
|*    Letzte Aenderung  TH 18.05.92
|*
*************************************************************************/

BOOL DateTime::IsBetween( const DateTime& rFrom,
                          const DateTime& rTo ) const
{
    if ( (*this >= rFrom) && (*this <= rTo) )
        return TRUE;
    else
        return FALSE;
}

/*************************************************************************
|*
|*    DateTime::operator >()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 18.05.92
|*    Letzte Aenderung  TH 18.05.92
|*
*************************************************************************/

BOOL DateTime::operator >( const DateTime& rDateTime ) const
{
    if ( (Date::operator>( rDateTime )) ||
         (Date::operator==( rDateTime ) && Time::operator>( rDateTime )) )
        return TRUE;
    else
        return FALSE;
}

/*************************************************************************
|*
|*    DateTime::operator <()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 18.05.92
|*    Letzte Aenderung  TH 18.05.92
|*
*************************************************************************/

BOOL DateTime::operator <( const DateTime& rDateTime ) const
{
    if ( (Date::operator<( rDateTime )) ||
         (Date::operator==( rDateTime ) && Time::operator<( rDateTime )) )
        return TRUE;
    else
        return FALSE;
}

/*************************************************************************
|*
|*    DateTime::operator >=()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 18.05.92
|*    Letzte Aenderung  TH 18.05.92
|*
*************************************************************************/

BOOL DateTime::operator >=( const DateTime& rDateTime ) const
{
    if ( (Date::operator>( rDateTime )) ||
         (Date::operator==( rDateTime ) && Time::operator>=( rDateTime )) )
        return TRUE;
    else
        return FALSE;
}

/*************************************************************************
|*
|*    DateTime::operator <=()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 18.05.92
|*    Letzte Aenderung  TH 18.05.92
|*
*************************************************************************/

BOOL DateTime::operator <=( const DateTime& rDateTime ) const
{
    if ( (Date::operator<( rDateTime )) ||
         (Date::operator==( rDateTime ) && Time::operator<=( rDateTime )) )
        return TRUE;
    else
        return FALSE;
}

/*************************************************************************
|*
|*    DateTime::GetSecFromDateTime()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 02.10.96
|*    Letzte Aenderung  TH 02.10.96
|*
*************************************************************************/

ULONG DateTime::GetSecFromDateTime( const Date& rDate ) const
{
    if ( Date::operator<( rDate ) )
        return 0;
    else
    {
        ULONG nSec = *this-rDate;
        nSec *= 24UL*60*60;
        long nHour = GetHour();
        long nMin  = GetMin();
        nSec += (nHour*3600)+(nMin*60)+GetSec();
        return nSec;
    }
}

/*************************************************************************
|*
|*    DateTime::GetSecFromDateTime()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 02.10.96
|*    Letzte Aenderung  TH 02.10.96
|*
*************************************************************************/

void DateTime::MakeDateTimeFromSec( const Date& rDate, ULONG nSec )
{
    long nDays = nSec / (24UL*60*60);
    ((Date*)this)->operator=( rDate );
    nSec -= nDays * (24UL*60*60);
    USHORT nMin = nSec / 60;
    nSec -= nMin * 60;
    ((Time*)this)->operator=( Time( 0, nMin, (USHORT)nSec ) );
    operator+=( nDays );
}

/*************************************************************************
|*
|*    DateTime::operator +=()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 02.10.96
|*    Letzte Aenderung  TH 02.10.96
|*
*************************************************************************/

DateTime& DateTime::operator +=( const Time& rTime )
{
    Time aTime = *this;
    aTime += rTime;
    USHORT nHours = aTime.GetHour();
    if ( aTime.GetTime() > 0 )
    {
        while ( nHours >= 24 )
        {
            Date::operator++();
            nHours -= 24;
        }
        aTime.SetHour( nHours );
    }
    else if ( aTime.GetTime() != 0 )
    {
        while ( nHours >= 24 )
        {
            Date::operator--();
            nHours -= 24;
        }
        Date::operator--();
        aTime = Time( 24, 0, 0 )+aTime;
    }
    Time::operator=( aTime );

    return *this;
}

/*************************************************************************
|*
|*    DateTime::operator -=()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 02.10.96
|*    Letzte Aenderung  TH 02.10.96
|*
*************************************************************************/

DateTime& DateTime::operator -=( const Time& rTime )
{
    Time aTime = *this;
    aTime -= rTime;
    USHORT nHours = aTime.GetHour();
    if ( aTime.GetTime() > 0 )
    {
        while ( nHours >= 24 )
        {
            Date::operator++();
            nHours -= 24;
        }
        aTime.SetHour( nHours );
    }
    else if ( aTime.GetTime() != 0 )
    {
        while ( nHours >= 24 )
        {
            Date::operator--();
            nHours -= 24;
        }
        Date::operator--();
        aTime = Time( 24, 0, 0 )+aTime;
    }
    Time::operator=( aTime );

    return *this;
}

/*************************************************************************
|*
|*    DateTime::operator+()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 02.10.96
|*    Letzte Aenderung  TH 02.10.96
|*
*************************************************************************/

DateTime operator +( const DateTime& rDateTime, long nDays )
{
    DateTime aDateTime( rDateTime );
    aDateTime += nDays;
    return aDateTime;
}

/*************************************************************************
|*
|*    DateTime::operator-()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 02.10.96
|*    Letzte Aenderung  TH 02.10.96
|*
*************************************************************************/

DateTime operator -( const DateTime& rDateTime, long nDays )
{
    DateTime aDateTime( rDateTime );
    aDateTime -= nDays;
    return aDateTime;
}

/*************************************************************************
|*
|*    DateTime::operator+()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 02.10.96
|*    Letzte Aenderung  TH 02.10.96
|*
*************************************************************************/

DateTime operator +( const DateTime& rDateTime, const Time& rTime )
{
    DateTime aDateTime( rDateTime );
    aDateTime += rTime;
    return aDateTime;
}

/*************************************************************************
|*
|*    DateTime::operator-()
|*
|*    Beschreibung      DATETIME.SDW
|*    Ersterstellung    TH 02.10.96
|*    Letzte Aenderung  TH 02.10.96
|*
*************************************************************************/

DateTime operator -( const DateTime& rDateTime, const Time& rTime )
{
    DateTime aDateTime( rDateTime );
    aDateTime -= rTime;
    return aDateTime;
}

/*************************************************************************
|*
|*    DateTime::operator +=( double )
|*
*************************************************************************/

DateTime& DateTime::operator +=( double fTimeInDays )
{
    double fInt, fFrac;
    if ( fTimeInDays < 0.0 )
    {
        fInt = SolarMath::ApproxCeil( fTimeInDays );
        fFrac = fInt <= fTimeInDays ? 0.0 : fTimeInDays - fInt;
    }
    else
    {
        fInt = SolarMath::ApproxFloor( fTimeInDays );
        fFrac = fInt >= fTimeInDays ? 0.0 : fTimeInDays - fInt;
    }
    Date::operator+=( long(fInt) );     // full days
    if ( fFrac )
    {
        Time aTime(0);  // default ctor calls system time, we don't need that
        fFrac *= 24UL * 60 * 60 * 1000;     // time expressed in milliseconds
        aTime.MakeTimeFromMS( long(fFrac) );    // method handles negative ms
        operator+=( aTime );
    }
    return *this;
}

/*************************************************************************
|*
|*    DateTime::operator +( double )
|*
*************************************************************************/

DateTime operator +( const DateTime& rDateTime, double fTimeInDays )
{
    DateTime aDateTime( rDateTime );
    aDateTime += fTimeInDays;
    return aDateTime;
}

/*************************************************************************
|*
|*    DateTime::operator -()
|*
*************************************************************************/

double operator -( const DateTime& rDateTime1, const DateTime& rDateTime2 )
{
    long nDays = (const Date&) rDateTime1 - (const Date&) rDateTime2;
    long nTime = rDateTime1.GetMSFromTime() - rDateTime2.GetMSFromTime();
    if ( nTime )
    {
        double fTime = double(nTime);
        fTime /= 24UL * 60 * 60 * 1000; // convert from milliseconds to fraction
        if ( nDays < 0 && fTime > 0.0 )
            fTime = 1.0 - fTime;
        return double(nDays) + fTime;
    }
    return double(nDays);
}
