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
    verify_testTypeName
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testTypeName()
    On Error GoTo errorHandler
    Dim b1 As Boolean
    Dim c1 As Byte
    Dim d1 As Date
    Dim d2 As Double
    Dim i1 As Integer
    Dim l1 As Long
    Dim s1 As String

    Dim TestIntSign%
    Dim TestLongSign&
    Dim TestSingleSign!
    Dim TestDoubleSign#
    Dim TestCurrSign@
    Dim TestStrSign$

    TestUtil.AssertEqual(TypeName(s1), "String",  "TypeName(s1)")
    TestUtil.AssertEqual(TypeName(b1), "Boolean", "TypeName(b1)")
    TestUtil.AssertEqual(TypeName(c1), "Byte",    "TypeName(c1)")
    TestUtil.AssertEqual(TypeName(d1), "Date",    "TypeName(d1)")
    TestUtil.AssertEqual(TypeName(d2), "Double",  "TypeName(d2)")
    TestUtil.AssertEqual(TypeName(i1), "Integer", "TypeName(i1)")
    TestUtil.AssertEqual(TypeName(l1), "Long",    "TypeName(l1)")

    ' tdf#129596 - Types of constant values
    TestUtil.AssertEqual(TypeName(32767),    "Integer", "TypeName(32767)")
    TestUtil.AssertEqual(TypeName(-32767),   "Integer", "TypeName(-32767)")
    TestUtil.AssertEqual(TypeName(1048575),  "Long",    "TypeName(1048575)")
    TestUtil.AssertEqual(TypeName(-1048575), "Long",    "TypeName(-1048575)")

    TestUtil.AssertEqual(TypeName(TestIntSign),    "Integer",  "TypeName(TestIntSign)")
    TestUtil.AssertEqual(TypeName(TestLongSign),   "Long",     "TypeName(TestLongSign)")
    TestUtil.AssertEqual(TypeName(TestSingleSign), "Single",   "TypeName(TestSingleSign)")
    TestUtil.AssertEqual(TypeName(TestDoubleSign), "Double",   "TypeName(TestDoubleSign)")
    TestUtil.AssertEqual(TypeName(TestCurrSign),   "Currency", "TypeName(TestCurrSign)")
    TestUtil.AssertEqual(TypeName(TestStrSign),    "String",   "TypeName(TestStrSign)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testTypeName", Err, Error$, Erl)
End Sub
