Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testReplace
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testReplace()

    TestUtilModule.TestInit

    Dim testName As String
    Dim srcStr, destStr, repStr, start, count, retStr
    testName = "Test Replace function"
    On Error GoTo errorHandler
    srcStr = "abcbcdBc"
    destStr = "bc"
    repStr = "ef"
    retStr = Replace(srcStr, destStr, repStr)
    TestUtilModule.AssertTrue(retStr = "aefefdBc", "common string:" & retStr)
    retStr = Replace("abcbcdbc", destStr, repStr)
    TestUtilModule.AssertTrue(retStr = "aefefdef", "expression string:" & retStr)
    retStr = Replace(srcStr, destStr, repStr, 1, -1, vbBinaryCompare)
    TestUtilModule.AssertTrue(retStr = "aefefdBc", "binary compare:" & retStr)
    retStr = Replace(srcStr, destStr, repStr, 1, -1, vbTextCompare)
    TestUtilModule.AssertTrue(retStr = "aefefdef", "text compare:" & retStr)
    retStr = Replace(srcStr, destStr, repStr, compare:=vbTextCompare)
    TestUtilModule.AssertTrue(retStr = "aefefdef", "text compare:" & retStr)
    retStr = Replace(srcStr, destStr, repStr, 3, -1, vbBinaryCompare)
    TestUtilModule.AssertTrue(retStr = "cefdBc", "start = 3:" & retStr)
    retStr = Replace(srcStr, destStr, repStr, 1, 2, vbBinaryCompare)
    TestUtilModule.AssertTrue(retStr = "aefefdBc", "count = 2: " & retStr)
    retStr = Replace(srcStr, destStr, repStr, 1, 0, vbBinaryCompare)
    TestUtilModule.AssertTrue(retStr = "abcbcdBc", "start = 1, count = 0, not support in Unix: " & retStr)

    ' tdf#132389 - case-insensitive operation for non-ASCII characters
    retStr = Replace("ABCabc", "b", "*", 1, 2, vbTextCompare)
    TestUtilModule.AssertTrue(retStr = "A*Ca*c", "case-insensitive ASCII: " & retStr)
    retStr = Replace("АБВабв", "б", "*", 1, 2, vbTextCompare)
    TestUtilModule.AssertTrue(retStr = "А*Ва*в", "case-insensitive non-ASCII: " & retStr)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
    TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

