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
    verify_testWeekDay
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testWeekDay()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Weekday(#6/7/2009#, vbMonday),  7, "Weekday(#6/7/2009#, vbMonday)")
    TestUtil.AssertEqual(Weekday(#7/7/2009#, vbMonday),  2, "Weekday(#7/7/2009#, vbMonday)")
    TestUtil.AssertEqual(Weekday(#8/7/2009#, vbMonday),  5, "Weekday(#8/7/2009#, vbMonday)")
    TestUtil.AssertEqual(Weekday(#12/7/2009#, vbMonday), 1, "Weekday(#12/7/2009#, vbMonday)")
    TestUtil.AssertEqual(Weekday(#6/7/2009#, vbSunday),  1, "Weekday(#6/7/2009#, vbSunday)")
    TestUtil.AssertEqual(Weekday(#6/7/2009#, 4),         5, "Weekday(#6/7/2009#, 4)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testWeekDay", Err, Error$, Erl)
End Sub
