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
    verify_testLBound
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testLBound()
    On Error GoTo errorHandler
    Dim MyArray(1 To 10, 5 To 15, 10 To 20)     ' Declare array variables.

    TestUtil.AssertEqual(LBound(MyArray, 1),  1, "LBound(MyArray, 1)")
    TestUtil.AssertEqual(LBound(MyArray, 3), 10, "LBound(MyArray, 3)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testLBound", Err, Error$, Erl)
End Sub
