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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"
#include <unotools/datetime.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
