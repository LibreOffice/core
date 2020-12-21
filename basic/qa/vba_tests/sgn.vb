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
    verify_SGN
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_SGN()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(sgn(0),    0, "sgn(0)")
    TestUtil.AssertEqual(sgn(-1),  -1, "sgn(-1)")
    TestUtil.AssertEqual(sgn(1),    1, "sgn(1)")
    TestUtil.AssertEqual(sgn(50),   1, "sgn(50)")
    TestUtil.AssertEqual(sgn(-50), -1, "sgn(-50)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_SGN", Err, Error$, Erl)
End Sub
