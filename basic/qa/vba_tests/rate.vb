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
    verify_testRATE
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testRATE()
    On Error GoTo errorHandler

    TestUtil.AssertEqualApprox(Rate(3, -5, 0, 16),        0.07,  1E-5, "Rate(3, -5, 0, 16)")
    TestUtil.AssertEqualApprox(Rate(3, -5, 0, 15),        0,     1E-5, "Rate(3, -5, 0, 15)")
    TestUtil.AssertEqualApprox(Rate(3, -5, 0, 30),        0.79,  1E-5, "Rate(3, -5, 0, 30)")
    TestUtil.AssertEqualApprox(Rate(3, -5, 0, 35),        1,     1E-5, "Rate(3, -5, 0, 35)")
    TestUtil.AssertEqualApprox(Rate(4, -300, 1000, 0, 0), 0.077, 1E-5, "Rate(4, -300, 1000, 0, 0)")
    TestUtil.AssertEqualApprox(Rate(4, -300, 1000, 0, 1), 0.14,  1E-5, "Rate(4, -300, 1000, 0, 1)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testRATE", Err, Error$, Erl)
End Sub
