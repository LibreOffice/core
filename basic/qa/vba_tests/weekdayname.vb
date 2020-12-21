'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testWeekDayName
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testWeekDayName()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(WeekdayName(1),                    "Sunday", "WeekdayName(1)")
    TestUtil.AssertEqual(WeekdayName(1, , vbSunday),        "Sunday", "WeekdayName(1, , vbSunday)")
    TestUtil.AssertEqual(WeekdayName(1, , vbMonday),        "Monday", "WeekdayName(1, , vbMonday)")
    TestUtil.AssertEqual(WeekdayName(2),                    "Monday", "WeekdayName(2)")
    TestUtil.AssertEqual(WeekdayName(2, True, vbMonday),    "Tue",    "WeekdayName(2, True, vbMonday)")
    TestUtil.AssertEqual(WeekdayName(2, True, vbTuesday),   "Wed",    "WeekdayName(2, True, vbTuesday)")
    TestUtil.AssertEqual(WeekdayName(2, True, vbWednesday), "Thu",    "WeekdayName(2, True, vbWednesday)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testWeekDayName", Err, Error$, Erl)
End Sub
