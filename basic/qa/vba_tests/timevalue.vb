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
    verify_testTimeValue
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testTimeValue()
    Dim timeVal As Date
    On Error GoTo errorHandler

    timeVal = "16:35:17"
    TestUtil.AssertEqual(TimeValue("4:35:17 PM"), timeVal, "TimeValue(""4:35:17 PM"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testTimeValue", Err, Error$, Erl)
End Sub
