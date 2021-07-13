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
    verify_testInStr
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testInStr()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(InStr(1, "somemoretext", "more"),      5, "InStr(1, ""somemoretext"", ""more"")")
    TestUtil.AssertEqual(InStr("somemoretext", "more"),         5, "InStr(""somemoretext"", ""more"")")
    TestUtil.AssertEqual(InStr("somemoretext", "somemoretext"), 1, "InStr(""somemoretext"", ""somemoretext"")")
    TestUtil.AssertEqual(InStr("somemoretext", "nothing"),      0, "InStr(""somemoretext"", ""nothing"")")

    Dim SearchString, SearchChar
    SearchString = "XXpXXpXXPXXP"   ' String to search in.
    SearchChar = "P"    ' Search for "P".
    TestUtil.AssertEqual(InStr(4, SearchString, SearchChar, 1), 6, "InStr(4, SearchString, SearchChar, 1)")
    TestUtil.AssertEqual(InStr(1, SearchString, SearchChar, 0), 9, "InStr(1, SearchString, SearchChar, 0)")
    TestUtil.AssertEqual(InStr(1, SearchString, "W"),           0, "InStr(1, SearchString, ""W"")")

    ' tdf#139840 - case-insensitive operation for non-ASCII characters
    TestUtil.AssertEqual(InStr(1, "α", "Α", 1), 1, "InStr(1, ""α"", ""Α"", 1)")
    ' tdf#139840 - German Eszett is uppercased to a two-character 'SS'.
    ' This test should fail after tdf#110003 has been fixed.
    TestUtil.AssertEqual(InStr(2, "Straße", "s", 1), 5, "InStr(2, ""Straße"", ""s"", 1)")

    ' Start position is greater than the length of the string being searched.
    TestUtil.AssertEqual(InStr(2, "α", "Α", 1), 0, "InStr(2, ""α"", ""Α"", 1)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testInStr", Err, Error$, Erl)
End Sub
