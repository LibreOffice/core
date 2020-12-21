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
    verify_testRGB
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testRGB()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(RGB(255, 0, 0),     255,      "RGB(255, 0, 0)")
    TestUtil.AssertEqual(RGB(75, 139, 203),  13339467, "RGB(75, 139, 203)")
    TestUtil.AssertEqual(RGB(255, 255, 255), 16777215, "RGB(255, 255, 255)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testRGB", Err, Error$, Erl)
End Sub
