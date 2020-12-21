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
    verify_testSIN
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testSIN()
    On Error GoTo errorHandler

    TestUtil.AssertEqualApprox(Sin(0.45), 0.43496553411123, 1E-14, "Sin(0.45)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testSIN", Err, Error$, Erl)
End Sub
