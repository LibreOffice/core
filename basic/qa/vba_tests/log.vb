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
    verify_testLog
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testLog()
    On Error GoTo errorHandler

    TestUtil.AssertEqualApprox(Log(86),               4.45434729625351, 1E-14, "Log(86)")
    TestUtil.AssertEqualApprox(Log(2.71828182845904), 1,                1E-14, "Log(2.71828182845904)")
    TestUtil.AssertEqual(Exp(Log(4)), 4, "Exp(Log(4))")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testLog", Err, Error$, Erl)
End Sub
