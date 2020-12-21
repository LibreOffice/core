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
    verify_testSQR
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testSQR()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Sqr(9), 3, "Sqr(9)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testSQR", Err, Error$, Erl)
End Sub
