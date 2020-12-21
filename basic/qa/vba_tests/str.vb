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
    verify_testSTR
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testSTR()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Str(459),     " 459",     "Str(459)")
    TestUtil.AssertEqual(Str(-459.65), "-459.65",  "Str(-459.65)")
    TestUtil.AssertEqual(Str(459.001), " 459.001", "Str(459.001)")
    TestUtil.AssertEqual(Str(0.24),    " .24",     "Str(0.24)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testSTR", Err, Error$, Erl)
End Sub
