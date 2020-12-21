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
    verify_testLCase
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testLCase()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(LCase("LOWERCASE"),  "lowercase",  "LCase(""LOWERCASE"")")
    TestUtil.AssertEqual(LCase("LowerCase"),  "lowercase",  "LCase(""LowerCase"")")
    TestUtil.AssertEqual(LCase("lowercase"),  "lowercase",  "LCase(""lowercase"")")
    TestUtil.AssertEqual(LCase("LOWER CASE"), "lower case", "LCase(""LOWER CASE"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testLCase", Err, Error$, Erl)
End Sub
