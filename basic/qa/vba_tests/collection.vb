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
    b.Add 3, "ß" ' uppercase German Eszett
    b.Add 4, "ẞ" ' lowercase German Eszett
    On Error GoTo 0

    On Error GoTo errorHandler

    ' tdf#144245 - check correct initialization of default value for optionals
    ' Without the fix in place, this test would have failed with
    ' - Expected: 2
    ' - Actual  : 3
    TestUtil.AssertEqual(a.Count, 2, "a.Count")

    ' tdf#144245 - German Eszett is uppercased to a two-character 'SS'.
    ' This test should fail after tdf#110003 has been fixed.
    TestUtil.AssertEqual(b.Count, 3, "b.Count")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCollection", Err, Error$, Erl)
End Sub
