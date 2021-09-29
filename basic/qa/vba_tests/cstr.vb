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

    ' tdf#143575 - round string to their nearest double representation
    ' Without the fix in place, this test would have failed with:
    ' - Expected: 691.2
    ' - Actual  : 691.2000000000001
    TestUtil.AssertEqual(CStr(691.2), "691.2", "CStr(691.2)")
    ' Without the fix in place, this test would have failed with:
    ' - Expected: 691.2
    ' - Actual  : 691.1999999999999
    TestUtil.AssertEqual(CStr(123.4 + 567.8), "691.2", "CStr(123.4 + 567.8)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCStr", Err, Error$, Erl)
End Sub
