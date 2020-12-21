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
    verify_testCHR
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCHR()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Chr(87),  "W", "Chr(87)")
    TestUtil.AssertEqual(Chr(105), "i", "Chr(105)")
    TestUtil.AssertEqual(Chr(35),  "#", "Chr(35)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCHR", Err, Error$, Erl)
End Sub
