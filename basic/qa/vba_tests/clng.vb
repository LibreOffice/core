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
    verify_testCLng
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCLng()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(CLng(-1.1), -1, "CLng(-1.1)")
    TestUtil.AssertEqual(CLng(-1.9), -2, "CLng(-1.9)")
    TestUtil.AssertEqual(CLng(0.2),   0, "CLng(0.2)")

REM    TestUtil.AssertEqual(CLng(0.5), 0, "CLng(0.5)")

REM    If the fraction is less than or equal to .5, the result will round down.
REM    If the fraction is greater than .5, the result will round up.

    TestUtil.AssertEqual(CLng(10.51),       11, "CLng(10.51)")
    TestUtil.AssertEqual(CLng("&H75FF"), 30207, "CLng(""&H75FF"")")
    TestUtil.AssertEqual(CLng("&H754"),   1876, "CLng(""&H754"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCLng", Err, Error$, Erl)
End Sub
