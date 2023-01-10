' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 0
Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_stringReplace
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_stringReplace()
    On Error GoTo errorHandler
    ' tdf#132389 - case-insensitive operation for non-ASCII characters
    Dim retStr
    retStr = Replace("ABCabc", "b", "*")
    TestUtil.AssertEqual(retStr, "A*Ca*c", "case-insensitive ASCII: " & retStr)
    retStr = Replace("АБВабв", "б", "*")
    TestUtil.AssertEqual(retStr, "А*Ва*в", "case-insensitive non-ASCII: " & retStr)

    ' tdf#141045 - different length of search and replace string. It is important
    ' that the search string starts with the original string in order to test the error.
    ' Without the fix in place, the string index calculations result in a crash.
    retStr = Replace("a", "abc", "ab")
    TestUtil.AssertEqual(retStr, "a", "different length of search and replace string: " & retStr)

    ' tdf#143081 - Without the fix in place, this test would have crashed here
    retStr = Replace("""Straße""", """", "&quot;")
    TestUtil.AssertEqual(retStr, "&quot;Straße&quot;", "replace doesn't crash: " & retStr)
    
    ' tdf#142487 - replace of special unicode characters.
    ' Without the fix in place, this test would have failed with:
    ' - Expected: Straßen
    ' - Actual  : Straßeen
    retStr = Replace("Straße", "e", "en")
    TestUtil.AssertEqual(retStr, "Straßen", "special unicode character: " & retStr)

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_stringReplace", Err, Error$, Erl)
End Sub
