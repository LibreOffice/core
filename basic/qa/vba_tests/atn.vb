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
    verify_testATN
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testATN()
    On Error GoTo errorHandler

    TestUtil.AssertEqualApprox(Atn(2),      1.10714871779409, 1E-14, "Atn(2)")
    TestUtil.AssertEqualApprox(Atn(2.51),   1.19166451926354, 1E-14, "Atn(2.51)")
    TestUtil.AssertEqualApprox(Atn(-3.25), -1.27229739520872, 1E-14, "Atn(-3.25)")
    TestUtil.AssertEqualApprox(Atn(210),    1.56603445802574, 1E-14, "Atn(210)")
    TestUtil.AssertEqual      (Atn(0),      0,                      "Atn(0)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testATN", Err, Error$, Erl)
End Sub
