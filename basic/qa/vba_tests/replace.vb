Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testReplace()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_testReplace() as String
    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim srcStr, destStr, repStr, start, count, retStr
    testName = "Test Replace function"
    On Error GoTo errorHandler
    srcStr = "abcbcdBc"
    destStr = "bc"
    repStr = "ef"
    retStr = Replace(srcStr, destStr, repStr)
    TestLog_ASSERT retStr = "aefefdBc", "common string:" & retStr
    retStr = Replace("abcbcdbc", destStr, repStr)
    TestLog_ASSERT retStr = "aefefdef", "expression string:" & retStr
    retStr = Replace(srcStr, destStr, repStr, 1, -1, vbBinaryCompare)
    TestLog_ASSERT retStr = "aefefdBc", "binary compare:" & retStr
    retStr = Replace(srcStr, destStr, repStr, 1, -1, vbTextCompare)
    TestLog_ASSERT retStr = "aefefdef", "text compare:" & retStr
    retStr = Replace(srcStr, destStr, repStr, compare:=vbTextCompare)
    TestLog_ASSERT retStr = "aefefdef", "text compare:" & retStr
    retStr = Replace(srcStr, destStr, repStr, 3, -1, vbBinaryCompare)
    TestLog_ASSERT retStr = "cefdBc", "start = 3:" & retStr
    retStr = Replace(srcStr, destStr, repStr, 1, 2, vbBinaryCompare)
    TestLog_ASSERT retStr = "aefefdBc", "count = 2: " & retStr
    retStr = Replace(srcStr, destStr, repStr, 1, 0, vbBinaryCompare)
    TestLog_ASSERT retStr = "abcbcdBc", "start = 1, count = 0, not support in Unix: " & retStr

    ' tdf#132389 - case-insensitive operation for non-ASCII characters
    retStr = Replace("ABCabc", "b", "*", 1, 2, vbTextCompare)
    TestLog_ASSERT retStr = "A*Ca*c", "case-insensitive ASCII: " & retStr
    retStr = Replace("АБВабв", "б", "*", 1, 2, vbTextCompare)
    TestLog_ASSERT retStr = "А*Ва*в", "case-insensitive non-ASCII: " & retStr

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testReplace = result

    Exit Function
errorHandler:
    TestLog_ASSERT (False), testName & ": hit error handler"
End Function

Sub TestLog_ASSERT(assertion As Boolean, Optional testId As String, Optional testComment As String)

    If assertion = True Then
        passCount = passCount + 1
    Else
        Dim testMsg As String
        If Not IsMissing(testId) Then
            testMsg = testMsg + " : " + testId
        End If
        If Not IsMissing(testComment) And Not (testComment = "") Then
            testMsg = testMsg + " (" + testComment + ")"
        End If

        result = result & Chr$(10) & " Failed: " & testMsg
        failCount = failCount + 1
    End If

End Sub
