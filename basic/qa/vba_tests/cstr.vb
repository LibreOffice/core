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
    verify_testCStr
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCStr()
    On Error GoTo errorHandler

    Dim n
    n = 437.324
    TestUtil.AssertEqual(CStr(n),   "437.324", "CStr(n)")
    TestUtil.AssertEqual(CStr(500), "500",     "CStr(500)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCStr", Err, Error$, Erl)
End Sub
