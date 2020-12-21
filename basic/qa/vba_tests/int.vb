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
    verify_testInt
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testInt()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Int(99.8),    99, "Int(99.8)")
    TestUtil.AssertEqual(Int(-99.8), -100, "Int(-99.8)")
    TestUtil.AssertEqual(Int(-99.2), -100, "Int(-99.2)")
    TestUtil.AssertEqual(Int(0.2),      0, "Int(0.2)")
    TestUtil.AssertEqual(Int(0),        0, "Int(0)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testInt", Err, Error$, Erl)
End Sub
