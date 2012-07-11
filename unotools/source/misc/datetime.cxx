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

#include <unotools/datetime.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>

//.........................................................................
namespace utl
{
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
