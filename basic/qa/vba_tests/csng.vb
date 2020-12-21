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
    verify_testCSng
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCSng()
    Dim nr1 As Single   'variables for test
    Dim nr2 As Double
    On Error GoTo errorHandler

    nr1 = 8.534535408
    TestUtil.AssertEqual(CSng(8.534535408), nr1, "CSng(8.534535408)")

    nr2 = 100.1234
    nr1 = 100.1234
    TestUtil.AssertEqual(CSng(nr2), nr1, "CSng(nr2)")

    nr1 = 0
    TestUtil.AssertEqual(CSng(0), nr1, "CSng(0)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCSng", Err, Error$, Erl)
End Sub
