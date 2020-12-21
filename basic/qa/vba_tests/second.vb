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
    verify_testSecond
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testSecond()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Second(37566.3),    0, "Second(37566.3)")
    TestUtil.AssertEqual(Second("4:35:17"), 17, "Second(""4:35:17"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testSecond", Err, Error$, Erl)
End Sub
