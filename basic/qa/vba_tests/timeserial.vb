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
    verify_testTimeSerial
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testTimeSerial()
    Dim timeVal As Date
    On Error GoTo errorHandler

rem    bug 114229
rem    timeVal = "5:45:00"
rem    TestUtil.AssertEqual(TimeSerial(12 - 6, -15, 0), timeVal, "TimeSerial(12 - 6, -15, 0)")

    timeVal = "12:30:00"
    TestUtil.AssertEqual(TimeSerial(12, 30, 0), timeVal, "TimeSerial(12, 30, 0)")

rem    timeVal = "11:30:00"
rem    TestUtil.AssertEqual(TimeSerial(10, 90, 0), timeVal, "TimeSerial(10, 90, 0)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testTimeSerial", Err, Error$, Erl)
End Sub
