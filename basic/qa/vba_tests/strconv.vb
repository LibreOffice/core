Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testStrConv()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_testStrConv() as String
    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim srcStr, retStr As String
    Dim i As Integer
    Dim x() As Byte
    srcStr = "abc EFG hij"
    testName = "Test StrConv function"
    On Error GoTo errorHandler

    retStr = StrConv(srcStr, vbUpperCase)
    TestLog_ASSERT retStr = "ABC EFG HIJ", "Converts the string to uppercase characters:" & retStr

    retStr = StrConv(srcStr, vbLowerCase)
    TestLog_ASSERT retStr = "abc efg hij", "Converts the string to lowercase characters:" & retStr

    retStr = StrConv(srcStr, vbProperCase)
    TestLog_ASSERT retStr = "Abc Efg Hij", "Converts the first letter of every word in string to uppercase:" & retStr

    'retStr = StrConv("ABCDEVB¥ì¥¹¥­¥å©`", vbWide)
    'MsgBox retStr
    'TestLog_ASSERT retStr = "£Á£Â£Ã£Ä£ÅVB¥ì¥¹¥­¥å©`", "Converts narrow (single-byte) characters in string to wide"

    'retStr = StrConv("£Á£Â£Ã£Ä£ÅVB¥ì¥¹¥­¥å©`", vbNarrow)
    'MsgBox retStr
    'TestLog_ASSERT retStr = "ABCDEVB¥ì¥¹¥­¥å©`", "Converts wide (double-byte) characters in string to narrow (single-byte) characters." & retStr

    'retStr = StrConv("¤Ï¤Ê¤Á¤ã¤ó", vbKatakana)
    'MsgBox retStr
    'TestLog_ASSERT retStr = "¥Ï¥Ê¥Á¥ã¥ó", "Converts Hiragana characters in string to Katakana characters.." & retStr

   ' retStr = StrConv("¥Ï¥Ê¥Á¥ã¥ó", vbHiragana)
    'MsgBox retStr
   ' TestLog_ASSERT retStr = "¤Ï¤Ê¤Á¤ã¤ó", "Converts Katakana characters in string to Hiragana characters.." & retStr

    'x = StrConv("ÉÏº£ÊÐABC", vbFromUnicode)
    'MsgBox retStr
    'TestLog_ASSERT UBound(x) = 8, "Converts the string from Unicode, the length is : " & UBound(x) + 1

   ' retStr = StrConv(x, vbUnicode)
    'MsgBox retStr
   ' TestLog_ASSERT retStr = "ÉÏº£ÊÐABC", "Converts the string to Unicode: " & retStr

    x = StrConv(srcStr, vbFromUnicode)
    TestLog_ASSERT UBound(x) = 10, "Converts the string from Unicode, the length is: " & UBound(x) + 1

    TestLog_ASSERT x(0) = 97, "Converts the string from Unicode, first char is: " & x(0)
    TestLog_ASSERT x(1) = 98, "Converts the string from Unicode, second char is: " & x(1)
    TestLog_ASSERT x(2) = 99, "Converts the string from Unicode, third char is: " & x(2)
    TestLog_ASSERT x(3) = 32, "Converts the string from Unicode, fourth char is: " & x(3)
    TestLog_ASSERT x(4) = 69, "Converts the string from Unicode, fifth char is: " & x(4)
    TestLog_ASSERT x(5) = 70, "Converts the string from Unicode, sixth char is: " & x(5)
    TestLog_ASSERT x(6) = 71, "Converts the string from Unicode, seventh char is: " & x(6)
    TestLog_ASSERT x(7) = 32, "Converts the string from Unicode, eighth char is: " & x(7)
    TestLog_ASSERT x(8) = 104, "Converts the string from Unicode, nineth char is: " & x(8)
    TestLog_ASSERT x(9) = 105, "Converts the string from Unicode, tenth char is: " & x(9)
    TestLog_ASSERT x(10) = 106, "Converts the string from Unicode, eleventh char is: " & x(10)

    ReDim x(Len(srcStr) - 1)
    For i = 1 To Len(srcStr)
        x(i - 1) = Asc(Mid(srcStr, i, 1))
    Next

    retStr = StrConv(x, vbUnicode)

    TestLog_ASSERT retStr = srcStr, "Converts the string to Unicode: " & retStr

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testStrConv = result

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
