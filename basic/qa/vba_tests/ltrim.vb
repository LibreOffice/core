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
    verify_testLTrim
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testLTrim()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(LTrim("   some text  "), "some text  ", "LTrim(""   some text  "")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testLTrim", Err, Error$, Erl)
End Sub
