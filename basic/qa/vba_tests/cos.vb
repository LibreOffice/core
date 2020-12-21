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
    verify_testCOS
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCOS()
    On Error GoTo errorHandler

    TestUtil.AssertEqualApprox(Cos(23), -0.532833020333398, 1E-14, "Cos(23)")
    TestUtil.AssertEqualApprox(Cos(0.2), 0.980066577841242, 1E-14, "Cos(0.2)")
    TestUtil.AssertEqualApprox(Cos(200), 0.487187675007006, 1E-14, "Cos(200)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCOS", Err, Error$, Erl)
End Sub
