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
    verify_byref_tdf145279
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_byref_tdf145279()

    On Error GoTo errorHandler

    ' tdf#145279 - test parameter passed ByRef
    ' Without the fix in place, this test would have failed with:
    ' - Expected: 4
    ' - Actual  : 2
    TestUtil.AssertEqual(TestOpt(), 4, "ByRef parameter")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_byref_tdf145279", Err, Error$, Erl)
End Sub

Function TestOpt(Optional a)
    If IsMissing(a) Then a = 2
    TestByRef(a)
    TestOpt = a
End Function

Function TestByRef(ByRef a)
    a = a + 2
End Function
