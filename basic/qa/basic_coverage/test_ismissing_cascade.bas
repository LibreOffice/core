' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testIsMissingCascade
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testIsMissingCascade()

    On Error GoTo errorHandler

    ' tdf#136143 - test cascading optionals in order to prevent type conversion errors, because
    ' optional arguments are of type SbxERROR and set to not fixed.
    TestUtil.AssertEqual(TestOpt(), 2, "Cascading optionals")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testIsMissingCascade", Err, Error$, Erl)
End Sub

Function TestOpt(Optional A)
    TestOpt = TestOptCascade(A)
End Function

Function TestOptCascade(Optional A)
    If IsMissing(A) Then A = 2
    TestOptCascade = A
End Function
