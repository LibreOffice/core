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

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testABS
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testABS()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Abs(-5),       5, "Abs(-5)")
    TestUtil.AssertEqual(Abs(5),        5, "Abs(5)")
    TestUtil.AssertEqual(Abs(-21.7), 21.7, "Abs(-21.7)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testABS", Err, Error$, Erl)
End Sub
