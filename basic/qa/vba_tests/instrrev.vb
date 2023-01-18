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
    TestUtil.AssertEqual(InStrRev(SearchString, "W", 1), 0, "InStrRev(SearchString, ""W"", 1)")

    ' tdf#143332 - case-insensitive operation for non-ASCII characters
    TestUtil.AssertEqual(InStrRev("α", "Α", -1, 1),      1, "InStrRev(""α"", ""Α"", -1, 1)")
    TestUtil.AssertEqual(InStrRev("abc", "d", -1, 1),    0, "InStrRev(""abc"", ""d"", -1, 1)")
    ' tdf#143332 - German Eszett is uppercased to a two-character 'SS'.
    ' This test should fail after tdf#110003 has been fixed.
    TestUtil.AssertEqual(InStrRev("Straße", "s", -1, 1), 5, "InStrRev(""Straße"", ""s"", -1, 1)")

    ' tdf#141474 keyword names need to match that of VBA
    Const vbBinaryCompare = 0, vbTextCompare = 1
    TestUtil.AssertEqual(InStrRev(stringMatch:="Star", stringCheck:="LibreOffice"),                                            0, "InStrRev(stringMatch:=""Star"", stringCheck:=""LibreOffice"")")
    TestUtil.AssertEqual(InStrRev(Start:=-1, stringMatch:="Libre", stringCheck:="LibreOfficeLibre"),                          12, "InStrRev(Start:=-1, stringMatch:=""Libre"", stringCheck:=""LibreOfficeLibre"")")
    TestUtil.AssertEqual(InStrRev(Start:=12, stringMatch:="Libre", stringCheck:="LibreOfficeLibre"),                           1, "InStrRev(Start:=12, stringMatch:=""Libre"", stringCheck:=""LibreOfficeLibre"")")
    TestUtil.AssertEqual(InStrRev(Compare:=vbBinaryCompare, Start:=12, stringMatch:="Libre", stringCheck:="LibreOfficeLibre"), 1, "InStrRev(Compare:=vbBinaryCompare, Start:=12, stringMatch:=""Libre"", stringCheck:=""LibreOfficeLibre"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testInStrRev", Err, Error$, Erl)
End Sub
