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

    TestUtil.AssertEqual(VarType(TestStr),           8, "VarType(TestStr)")
    TestUtil.AssertEqual(VarType(TestBoo),           11, "VarType(TestBoo)")
    TestUtil.AssertEqual(VarType(TestDouble),        5, "VarType(TestDouble)")
    TestUtil.AssertEqual(VarType(TestLong),          3, "VarType(TestLong)")
    TestUtil.AssertEqual(VarType(TestInt),           2, "VarType(TestInt)")
    TestUtil.AssertEqual(VarType(TestDateTime),      7, "VarType(TestDateTime)")
    TestUtil.AssertEqual(VarType(TestSingle),        4, "VarType(TestSingle)")
    TestUtil.AssertEqual(VarType(TestCurr),          6, "VarType(TestCurr)")
    TestUtil.AssertEqual(VarType(TestIntSign),       2, "VarType(TestIntSign)")
    TestUtil.AssertEqual(VarType(TestLongSign),      3, "VarType(TestLongSign)")
    TestUtil.AssertEqual(VarType(TestSingleSign),    4, "VarType(TestSingleSign)")
    TestUtil.AssertEqual(VarType(TestDoubleSign),    5, "VarType(TestDoubleSign)")
    TestUtil.AssertEqual(VarType(TestCurrSign),      6, "VarType(TestCurrSign)")
    TestUtil.AssertEqual(VarType(vbEmpty),           0, "VarType(vbEmpty)")
    TestUtil.AssertEqual(VarType(vbNull),            1, "VarType(vbNull)")
    TestUtil.AssertEqual(VarType(vbInteger),         2, "VarType(vbInteger)")
    TestUtil.AssertEqual(VarType(vbLong),            3, "VarType(vbLong)")
    TestUtil.AssertEqual(VarType(vbSingle),          4, "VarType(vbSingle)")
    TestUtil.AssertEqual(VarType(vbDouble),          5, "VarType(vbDouble)")
    TestUtil.AssertEqual(VarType(vbCurrency),        6, "VarType(vbCurrency)")
    TestUtil.AssertEqual(VarType(vbDate),            7, "VarType(vbDate)")
    TestUtil.AssertEqual(VarType(vbString),          8, "VarType(vbString)")
    TestUtil.AssertEqual(VarType(vbObject),          9, "VarType(vbObject)")
    TestUtil.AssertEqual(VarType(vbError),           10, "VarType(vbError)")
    TestUtil.AssertEqual(VarType(vbBoolean),         11, "VarType(vbBoolean)")
    TestUtil.AssertEqual(VarType(vbVariant),         12, "VarType(vbVariant)")
    TestUtil.AssertEqual(VarType(vbDataObject),      13, "VarType(vbDataObject)")
    TestUtil.AssertEqual(VarType(vbDecimal),         14, "VarType(vbDecimal)")
    TestUtil.AssertEqual(VarType(vbByte),            17, "VarType(vbByte)")
    TestUtil.AssertEqual(VarType(vbLongLong),        20, "VarType(vbLongLong)")
    TestUtil.AssertEqual(VarType(vbUserDefinedType), 36, "VarType(vbUserDefinedType)")
    TestUtil.AssertEqual(VarType(vbArray),           8192, "VarType(vbArray)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testVarType", Err, Error$, Erl)
End Sub
