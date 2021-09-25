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
    verify_testConstants
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testConstants()
    On Error GoTo errorHandler

    ' vbNewLine is the same as vbCrLf on Windows, and the same as vbLf on other OSes
    If GetGuiType() = 1 Then
        TestUtil.AssertEqual(vbNewline, vbCrLf, "vbNewline")
    Else
        TestUtil.AssertEqual(vbNewLine, vbLf, "vbNewline")
    End If

    ' tdf#131563 - check for vba color constants
    ' See https://docs.microsoft.com/en-us/office/vba/language/reference/user-interface-help/color-constants
    TestUtil.AssertEqual(vbBlack, &H0&, "vbBlack")
    TestUtil.AssertEqual(vbRed, &HFF&, "vbRed")
    TestUtil.AssertEqual(vbGreen, &HFF00&, "vbGreen")
    TestUtil.AssertEqual(vbYellow, &HFFFF&, "vbYellow")
    TestUtil.AssertEqual(vbBlue, &HFF0000&, "vbBlue")
    TestUtil.AssertEqual(vbMagenta, &HFF00FF&, "vbMagenta")
    TestUtil.AssertEqual(vbCyan, &HFFFF00&, "vbCyan")
    TestUtil.AssertEqual(vbWhite, &HFFFFFF&, "vbWhite")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testConstants", Err, Error$, Erl)
End Sub
