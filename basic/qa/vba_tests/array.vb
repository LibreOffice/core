'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Rem Attribute VBA_ModuleType=VBAModule
Option VBASupport 1
Option Explicit

Type MyType
    ax(3) As Integer
    bx As Double
End Type

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testARRAY
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testARRAY()
    On Error GoTo errorHandler

    Dim a
    a = Array(10, 20, 30)
    TestUtil.AssertEqual(a(0), 10, "a(0)")
    TestUtil.AssertEqual(a(1), 20, "a(1)")
    TestUtil.AssertEqual(a(2), 30, "a(2)")

    Dim MyWeek
    MyWeek = Array("Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun")
    TestUtil.AssertEqual(MyWeek(1), "Tue", "MyWeek(1)")
    TestUtil.AssertEqual(MyWeek(3), "Thu", "MyWeek(3)")

    Dim mt As MyType
    mt.ax(0) = 42
    mt.ax(1) = 43
    mt.bx = 3.14
    TestUtil.AssertEqual(mt.ax(1), 43, "mt.ax(1)")
    TestUtil.AssertEqual(mt.bx,  3.14, "mt.bx")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testARRAY", Err, Error$, Erl)
End Sub
