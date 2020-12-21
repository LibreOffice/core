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
    verify_testUBound
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testUBound()
    On Error GoTo errorHandler
    Dim A(1 To 100, 0 To 3, -3 To 4)

    TestUtil.AssertEqual(UBound(A, 1), 100, "UBound(A, 1)")
    TestUtil.AssertEqual(UBound(A, 2),   3, "UBound(A, 2)")
    TestUtil.AssertEqual(UBound(A, 3),   4, "UBound(A, 3)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testUBound", Err, Error$, Erl)
End Sub
