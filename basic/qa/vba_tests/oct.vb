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
    verify_testOct
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testOct()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Oct(4),   "4",   "Oct(4)")
    TestUtil.AssertEqual(Oct(8),   "10",  "Oct(8)")
    TestUtil.AssertEqual(Oct(459), "713", "Oct(459)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testOct", Err, Error$, Erl)
End Sub
