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
    verify_roundCompatibility
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_roundCompatibility()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Round(0.12335,4),   0.1234,  "Round(0.12335,4)")
    TestUtil.AssertEqual(Round(0.12345,4),   0.1234,  "Round(0.12345,4)")
    TestUtil.AssertEqual(Round(0.12355,4),   0.1236,  "Round(0.12355,4)")
    TestUtil.AssertEqual(Round(0.12365,4),   0.1236,  "Round(0.12365,4)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_roundCompatibility", Err, Error$, Erl)
End Sub
