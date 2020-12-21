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
    verify_testQBcolor
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testQBcolor()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(QBColor(0), 0,        "QBColor(0)")
    TestUtil.AssertEqual(QBColor(1), 8388608,  "QBColor(1)")
    TestUtil.AssertEqual(QBColor(2), 32768,    "QBColor(2)")
    TestUtil.AssertEqual(QBColor(3), 8421376,  "QBColor(3)")
    TestUtil.AssertEqual(QBColor(4), 128,      "QBColor(4)")
    TestUtil.AssertEqual(QBColor(5), 8388736,  "QBColor(5)")
    TestUtil.AssertEqual(QBColor(6), 32896,    "QBColor(6)")
    TestUtil.AssertEqual(QBColor(7), 12632256, "QBColor(7)")
    TestUtil.AssertEqual(QBColor(8), 8421504,  "QBColor(8)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testQBcolor", Err, Error$, Erl)
End Sub
