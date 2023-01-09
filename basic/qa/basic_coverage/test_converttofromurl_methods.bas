' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testConvertToFromUrl
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testConvertToFromUrl
    On Error GoTo errorHandler

    TestUtil.AssertEqual( ConvertToUrl( ConvertFromUrl("") ), "", "ConvertToUrl( ConvertFromUrl("") )")

    ' tdf#152917: Without the fix in place, this test would have failed with
    ' Failed: ConvertFromUrl("file:///foo/bar/test.txt") returned , expected /foo/bar/test.txt
    If (GetGUIType() <> 1) Then
        'Linux
        TestUtil.AssertEqual( ConvertFromUrl("file:///foo/bar/test.txt"), "/foo/bar/test.txt", "ConvertFromUrl(""file:///foo/bar/test.txt"")")
    Else
        'Windows
        TestUtil.AssertEqual( ConvertFromUrl("file://foo/bar/test.txt"), "\\foo\bar\test.txt", "ConvertFromUrl(""file://foo/bar/test.txt"")")
    End If

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testConvertToFromUrl", Err, Error$, Erl)
End Sub
