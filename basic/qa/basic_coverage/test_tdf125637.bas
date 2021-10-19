'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_tdf125637
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_tdf125637
    On Error GoTo errorHandler

    ' tdf#125637 - correctly hand names ending with an underscore character at the end of the line
    Dim test As Long
    Dim test_ As Long
    test_ = 1234
    test = test_

    ' Without the fix in place, this test would have failed with:
    ' - Expected: 1234
    ' - Actual  : 0
    TestUtil.AssertEqual(test, 1234, "Assignment of the variable failed (tdf#125637)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_tdf125637", Err, Error$, Erl)
End Sub
