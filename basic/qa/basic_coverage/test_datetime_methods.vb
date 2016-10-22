'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    dim aVector as Variant
    aVector = Array( Date(), Time(), "Hello", -3.14)
    ' CDateFromUnoDate
    ' CDateTomUnoDate
    ' CDateFromUnoTime
    ' CDateToUnoTime
    ' CDateFromUnoDateTime
    ' CDateToUnoDateTime
    ' CDateFromIso CDateToIso
    If ( CDateToIso( CDateFromIso("20161016") ) <> "20161016" ) Then
        doUnitTest = 0
    ' DATE DATEDIFF DATEADD
    Else If ( DateDiff( "d", aVector(0), DateAdd("d", 1, aVector(0)) ) <> 1 ) Then
        doUnitTest = 0
    ' DATEPART  DAY
    Else If ( DatePart( "d", aVector(0) ) <> Day( aVector(0) ) ) Then
        doUnitTest = 0
    ' DATESERIAL DATEVALUE
    Else If ( DateSerial( 2016, 10, 16 ) <> DateValue("October 16, 2016") ) Then
        doUnitTest = 0
    ' FormatDateTime
    ' HOUR
    Else If ( Hour(aVector(1)) <> Hour(aVector(1)) ) Then
        doUnitTest = 0
    ' MINUTE
    Else If ( Minute(aVector(1)) <> Minute(aVector(1)) ) Then
        doUnitTest = 0
    ' MONTH
    Else If ( DatePart( "m", aVector(0) ) <> Month( aVector(0) ) ) Then
        doUnitTest = 0
    ' MONTHNAME
    'Else If ( CDate( MonthName(10)&" 16, 2016" )  <> DateSerial(2016, 10, 16) ) Then
    '    doUnitTest = 0
    ' NOW TIMEVALUE
    Else If ( Now() < aVector(0) + TimeValue(aVector(1)) ) Then
        doUnitTest = 0
    ' SECOND
    Else If ( Second(aVector(1)) <> Second(aVector(1)) ) Then
        doUnitTest = 0
    ' TIMER             max value = 24*3600
    Else If ( Timer() > 86400 ) Then
        doUnitTest = 0
    ' TIMESERIAL TIMEVALUE
    Else If ( TimeSerial(13,54,48) <> TImeValue("13:54:48") ) Then
        doUnitTest = 0
    ' WEEKDAY WEEKDAYNAME
    'Else If ( WeekdayName( Weekday(aVector(0) ) ) <> WeekDayName( DatePart( "w", aVector(0) ) ) ) Then
    '    doUnitTest = 0
    ' YEAR
    Else If ( DatePart( "yyyy", aVector(0) ) <> Year( aVector(0) ) ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    'End If
    'End If
End Function
