/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datetime.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:27:10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#ifndef _UNOTOOLS_DATETIME_HXX_
#include <unotools/datetime.hxx>
#endif

#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

//.........................................................................
namespace utl
{
//.........................................................................

//------------------------------------------------------------------
void typeConvert(const Time& _rTime, starutil::Time& _rOut)
{
    _rOut.Hours = _rTime.GetHour();
    _rOut.Minutes = _rTime.GetMin();
    _rOut.Seconds = _rTime.GetSec();
    _rOut.HundredthSeconds = _rTime.Get100Sec();
}

//------------------------------------------------------------------
void typeConvert(const starutil::Time& _rTime, Time& _rOut)
{
    _rOut = Time(_rTime.Hours, _rTime.Minutes, _rTime.Seconds, _rTime.HundredthSeconds);
}

//------------------------------------------------------------------
void typeConvert(const Date& _rDate, starutil::Date& _rOut)
{
    _rOut.Day = _rDate.GetDay();
    _rOut.Month = _rDate.GetMonth();
    _rOut.Year = _rDate.GetYear();
}

//------------------------------------------------------------------
void typeConvert(const starutil::Date& _rDate, Date& _rOut)
{
    _rOut = Date(_rDate.Day, _rDate.Month, _rDate.Year);
}

//------------------------------------------------------------------
void typeConvert(const DateTime& _rDateTime, starutil::DateTime& _rOut)
{
    _rOut.Year = _rDateTime.GetYear();
    _rOut.Month = _rDateTime.GetMonth();
    _rOut.Day = _rDateTime.GetDay();
    _rOut.Hours = _rDateTime.GetHour();
    _rOut.Minutes = _rDateTime.GetMin();
    _rOut.Seconds = _rDateTime.GetSec();
    _rOut.HundredthSeconds = _rDateTime.Get100Sec();
}

//------------------------------------------------------------------
void typeConvert(const starutil::DateTime& _rDateTime, DateTime& _rOut)
{
    Date aDate(_rDateTime.Day, _rDateTime.Month, _rDateTime.Year);
    Time aTime(_rDateTime.Hours, _rDateTime.Minutes, _rDateTime.Seconds, _rDateTime.HundredthSeconds);
    _rOut = DateTime(aDate, aTime);
}

//-------------------------------------------------------------------------
sal_Bool    operator ==(const starutil::DateTime& _rLeft, const starutil::DateTime& _rRight)
{
    return ( _rLeft.HundredthSeconds == _rRight.HundredthSeconds) &&
    ( _rLeft.Seconds == _rRight.Seconds) &&
    ( _rLeft.Minutes == _rRight.Minutes) &&
    ( _rLeft.Hours == _rRight.Hours) &&
    ( _rLeft.Day == _rRight.Day) &&
    ( _rLeft.Month == _rRight.Month) &&
    ( _rLeft.Year == _rRight.Year) ;
}

//-------------------------------------------------------------------------
sal_Bool    operator ==(const starutil::Date& _rLeft, const starutil::Date& _rRight)
{
    return ( _rLeft.Day == _rRight.Day) &&
    ( _rLeft.Month == _rRight.Month) &&
    ( _rLeft.Year == _rRight.Year) ;
}

//-------------------------------------------------------------------------
sal_Bool    operator ==(const starutil::Time& _rLeft, const starutil::Time& _rRight)
{
    return ( _rLeft.HundredthSeconds == _rRight.HundredthSeconds) &&
    ( _rLeft.Seconds == _rRight.Seconds) &&
    ( _rLeft.Minutes == _rRight.Minutes) &&
    ( _rLeft.Hours == _rRight.Hours) ;
}

//.........................................................................
}   // namespace utl
//.........................................................................

