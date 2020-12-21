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
    verify_testInStrRev
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testInStrRev()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(InStrRev("somemoretext", "more", -1),     5, "InStrRev(""somemoretext"", ""more"", -1)")
    TestUtil.AssertEqual(InStrRev("somemoretext", "more"),         5, "InStrRev(""somemoretext"", ""more"")")
    TestUtil.AssertEqual(InStrRev("somemoretext", "somemoretext"), 1, "InStrRev(""somemoretext"", ""somemoretext"")")
    TestUtil.AssertEqual(InStrRev("somemoretext", "nothing"),      0, "InStrRev(""somemoretext"", ""nothing"")")

    Dim SearchString, SearchChar
    SearchString = "XXpXXpXXPXXP"   ' String to search in.
    SearchChar = "P"    ' Search for "P".
    TestUtil.AssertEqual(InStrRev(SearchString, SearchChar, 4, 1),   3, "InStrRev(SearchString, SearchChar, 4, 1)")
    TestUtil.AssertEqual(InStrRev(SearchString, SearchChar, -1, 0), 12, "InStrRev(SearchString, SearchChar, -1, 0)")
    TestUtil.AssertEqual(InStrRev(SearchString, "W", 1),             0, "InStrRev(SearchString, ""W"", 1)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testInStrRev", Err, Error$, Erl)
End Sub
