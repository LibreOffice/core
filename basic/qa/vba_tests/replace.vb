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
    verify_testReplace
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testReplace()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Replace("abcbcdBc", "bc", "ef"),                         "aefefdBc", "Replace(""abcbcdBc"", ""bc"", ""ef"")")
    TestUtil.AssertEqual(Replace("abcbcdbc", "bc", "ef"),                         "aefefdef", "Replace(""abcbcdbc"", ""bc"", ""ef"")")
    TestUtil.AssertEqual(Replace("abcbcdBc", "bc", "ef", 1, -1, vbBinaryCompare), "aefefdBc", "Replace(""abcbcdBc"", ""bc"", ""ef"", 1, -1, vbBinaryCompare)")
    TestUtil.AssertEqual(Replace("abcbcdBc", "bc", "ef", 1, -1, vbTextCompare),   "aefefdef", "Replace(""abcbcdBc"", ""bc"", ""ef"", 1, -1, vbTextCompare)")
    TestUtil.AssertEqual(Replace("abcbcdBc", "bc", "ef", compare:=vbTextCompare), "aefefdef", "Replace(""abcbcdBc"", ""bc"", ""ef"", compare:=vbTextCompare)")
    TestUtil.AssertEqual(Replace("abcbcdBc", "bc", "ef", 3, -1, vbBinaryCompare), "cefdBc",   "Replace(""abcbcdBc"", ""bc"", ""ef"", 3, -1, vbBinaryCompare)")
    TestUtil.AssertEqual(Replace("abcbcdBc", "bc", "ef", 1, 2, vbBinaryCompare),  "aefefdBc", "Replace(""abcbcdBc"", ""bc"", ""ef"", 1, 2, vbBinaryCompare)")
    TestUtil.AssertEqual(Replace("abcbcdBc", "bc", "ef", 1, 0, vbBinaryCompare),  "abcbcdBc", "Replace(""abcbcdBc"", ""bc"", ""ef"", 1, 0, vbBinaryCompare)") ' not support in Unix

    ' tdf#132389 - case-insensitive operation for non-ASCII characters
    TestUtil.AssertEqual(Replace("ABCabc", "b", "*", 1, 2, vbTextCompare), "A*Ca*c", "Replace(""ABCabc"", ""b"", ""*"", 1, 2, vbTextCompare)")
    TestUtil.AssertEqual(Replace("АБВабв", "б", "*", 1, 2, vbTextCompare), "А*Ва*в", "Replace(""АБВабв"", ""б"", ""*"", 1, 2, vbTextCompare)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testReplace", Err, Error$, Erl)
End Sub
