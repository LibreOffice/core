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
    verify_testTrim
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testTrim()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Trim("   some text  "), "some text", "Trim(""   some text  "")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testTrim", Err, Error$, Erl)
End Sub
