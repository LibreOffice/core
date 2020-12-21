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
    verify_testMonthName
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testMonthName()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(MonthName(2),        "February", "MonthName(2)")
    TestUtil.AssertEqual(MonthName(2, True),  "Feb",      "MonthName(2, True)")
    TestUtil.AssertEqual(MonthName(2, False), "February", "MonthName(2, False)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testMonthName", Err, Error$, Erl)
End Sub
