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
    verify_testFix
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testFix()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Fix(12.34), 12, "Fix(12.34)")
    TestUtil.AssertEqual(Fix(12.99), 12, "Fix(12.99)")
    TestUtil.AssertEqual(Fix(-8.4),  -8, "Fix(-8.4)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testFix", Err, Error$, Erl)
End Sub
