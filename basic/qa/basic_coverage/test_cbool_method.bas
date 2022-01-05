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
    verify_CBool
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_CBool()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(CBool(3),  True,  "CBool(3)")

    TestUtil.AssertEqual(CBool(1>2 Xor 44),                True, "CBool(1>2 Xor 44)")
    TestUtil.AssertEqual(CBool(expression:="15" /2 -7.5), False, "CBool(expression:=""15"" /2 -7.5)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_CBool", Err, Error$, Erl)
End Sub
