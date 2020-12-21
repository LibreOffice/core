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
    verify_testVarType
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testVarType()
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim TestInt As Integer
    Dim TestLong As Long
    Dim TestDouble As Double
    Dim TestBoo As Boolean
    On Error GoTo errorHandler

    TestUtil.AssertEqual(VarType(TestStr),      8, "VarType(TestStr)")
    TestUtil.AssertEqual(VarType(TestBoo),     11, "VarType(TestBoo)")
    TestUtil.AssertEqual(VarType(TestDouble),   5, "VarType(TestDouble)")
    TestUtil.AssertEqual(VarType(TestLong),     3, "VarType(TestLong)")
    TestUtil.AssertEqual(VarType(TestInt),      2, "VarType(TestInt)")
    TestUtil.AssertEqual(VarType(TestDateTime), 7, "VarType(TestDateTime)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testVarType", Err, Error$, Erl)
End Sub
