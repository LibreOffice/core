Option VBASupport 0

Function doUnitTest() As String
    TestUtil.TestInit
    verify_stringReplace
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_stringReplace()
    On Error GoTo errorHandler
    ' tdf#132389 - case-insensitive operation for non-ASCII characters
    retStr = Replace("ABCabc", "b", "*")
    TestUtil.AssertEqual(retStr, "A*Ca*c", "case-insensitive ASCII: " & retStr)
    retStr = Replace("АБВабв", "б", "*")
    TestUtil.AssertEqual(retStr, "А*Ва*в", "case-insensitive non-ASCII: " & retStr)

    ' tdf#141045 - different length of search and replace string. It is important
    ' that the search string starts with the original string in order to test the error.
    ' Without the fix in place, the string index calculations result in a crash.
    retStr = Replace("a", "abc", "ab")
    TestUtil.AssertEqual(retStr, "a", "different length of search and replace string: " & retStr)

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_stringReplace", Err, Error$, Erl)
End Sub
