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
    verify_testCollection
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCollection()

    Dim a As Collection
    Dim b As Collection

    On Error Resume Next
    Set a = New Collection
    a.Add 1, "D"
    a.Add 2, "d"
    a.Add 3, "Д" ' uppercase Cyrillic script De
    a.Add 4, "д" ' lowercase Cyrillic script De
    On Error GoTo 0

    On Error Resume Next
    Set b = New Collection
    b.Add 1, "SS"
    b.Add 2, "ss"
    b.Add 3, "ẞ" ' uppercase German Eszett
    b.Add 4, "ß" ' lowercase German Eszett
    On Error GoTo 0

    On Error GoTo errorHandler

    ' tdf#144245 - case-insensitive operation for non-ASCII characters
    ' Without the fix in place, this test would have failed with
    ' - Expected: 2
    ' - Actual  : 3
    TestUtil.AssertEqual(a.Count, 2, "a.Count")

    ' tdf#144245 - case-insensitive operation for non-ASCII item access
    ' Without the fix in place, this test would have failed with
    ' - Expected: 1 for d, 3 for lowercase Cyrillic script De (д)
    ' - Actual  : 2 for d, 4 for lowercase Cyrillic script De (д)
    TestUtil.AssertEqual(a.Item("D"), 1, "a.Item(""D"")")
    TestUtil.AssertEqual(a.Item("d"), 1, "a.Item(""d"")")
    TestUtil.AssertEqual(a.Item("Д"), 3, "a.Item(""Д"")")
    TestUtil.AssertEqual(a.Item("д"), 3, "a.Item(""д"")")

    ' tdf#144245 - German Eszett is uppercased to a two-character 'SS'.
    ' This test should fail after tdf#110003 has been fixed since the lowercase and the uppercase
    ' German Eszett should be matched to the same index.
    TestUtil.AssertEqual(b.Count, 3, "b.Count")
    ' After the fix of tdf#110003
    ' TestUtil.AssertEqual(b.Count, 2, "b.Count")

    TestUtil.AssertEqual(a.Item("SS"), 1, "a.Item(""SS"")")
    TestUtil.AssertEqual(a.Item("ss"), 1, "a.Item(""ss"")")
    TestUtil.AssertEqual(a.Item("ẞ"), 3, "a.Item(""ẞ"")")
    TestUtil.AssertEqual(a.Item("ß"), 4, "a.Item(""ß"")")
    ' After the fix of tdf#110003
    ' TestUtil.AssertEqual(a.Item("ß"), 3, "a.Item(""ß"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCollection", Err, Error$, Erl)
End Sub
