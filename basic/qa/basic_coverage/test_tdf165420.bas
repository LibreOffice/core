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
    verify_tdf165420()
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_tdf165420()
    On Error GoTo errorHandler

    Dim nHandlerInvocations As Integer

    ' Calling 'Shell' function with an empty argument must not crash
    Shell(Empty)
    Shell("")
    Shell(" ")

    TestUtil.AssertEqual(nHandlerInvocations, 3, "nHandlerInvocations")

    Exit Sub

errorHandler:
    TestUtil.AssertEqual(Err, 5, "Err") ' Expected: Invalid procedure call
    nHandlerInvocations = nHandlerInvocations + 1
    Resume Next
End Sub
