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
    Dim TestSingle As Single
    Dim TestCurr As Currency

    Dim TestIntSign%
    Dim TestLongSign&
    Dim TestSingleSign!
    Dim TestDoubleSign#
    Dim TestCurrSign@
    Dim TestStrSign$
    On Error GoTo errorHandler

    TestUtil.AssertEqual(vbInteger,  2,  "vbInteger")
    TestUtil.AssertEqual(vbLong,     3,  "vbLong")
    TestUtil.AssertEqual(vbSingle,   4,  "vbSingle")
    TestUtil.AssertEqual(vbDouble,   5,  "vbDouble")
    TestUtil.AssertEqual(vbCurrency, 6,  "vbCurrency")
    TestUtil.AssertEqual(vbDate,     7,  "vbDate")
    TestUtil.AssertEqual(vbString,   8,  "vbString")
    TestUtil.AssertEqual(vbBoolean,  11, "vbBoolean")

    TestUtil.AssertEqual(VarType(TestStr),        vbString,   "VarType(TestStr)")
    TestUtil.AssertEqual(VarType(TestBoo),        vbBoolean,  "VarType(TestBoo)")
    TestUtil.AssertEqual(VarType(TestDouble),     vbDouble,   "VarType(TestDouble)")
    TestUtil.AssertEqual(VarType(TestLong),       vbLong,     "VarType(TestLong)")
    TestUtil.AssertEqual(VarType(TestInt),        vbInteger,  "VarType(TestInt)")
    TestUtil.AssertEqual(VarType(TestDateTime),   vbDate,     "VarType(TestDateTime)")
    TestUtil.AssertEqual(VarType(TestSingle),     vbSingle,   "VarType(TestSingle)")
    TestUtil.AssertEqual(VarType(TestCurr),       vbCurrency, "VarType(TestCurr)")

    TestUtil.AssertEqual(VarType(TestIntSign),    vbInteger,  "VarType(TestIntSign)")
    TestUtil.AssertEqual(VarType(TestLongSign),   vbLong,     "VarType(TestLongSign)")
    TestUtil.AssertEqual(VarType(TestSingleSign), vbSingle,   "VarType(TestSingleSign)")
    TestUtil.AssertEqual(VarType(TestDoubleSign), vbDouble,   "VarType(TestDoubleSign)")
    TestUtil.AssertEqual(VarType(TestCurrSign),   vbCurrency, "VarType(TestCurrSign)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testVarType", Err, Error$, Erl)
End Sub
