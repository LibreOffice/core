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
    verify_testLen
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testLen()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Len("sometext"),  8, "Len(""sometext"")")
    TestUtil.AssertEqual(Len("some text"), 9, "Len(""some text"")")
    TestUtil.AssertEqual(Len(""),          0, "Len("""")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testLen", Err, Error$, Erl)
End Sub
