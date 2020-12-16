Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testStrConv
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testStrConv()

    TestUtilModule.TestInit

    Dim testName As String
    Dim srcStr, retStr As String
    Dim x() As Byte
    srcStr = "abc EFG hij"
    testName = "Test StrConv function"
    On Error GoTo errorHandler

    retStr = StrConv(srcStr, vbUpperCase)
    'MsgBox retStr
    TestUtilModule.AssertTrue(retStr = "ABC EFG HIJ", "Converts the string to uppercase characters:" & retStr)

    retStr = StrConv(srcStr, vbLowerCase)
    'MsgBox retStr
    TestUtilModule.AssertTrue(retStr = "abc efg hij", "Converts the string to lowercase characters:" & retStr)

    retStr = StrConv(srcStr, vbProperCase)
    'MsgBox retStr
    TestUtilModule.AssertTrue(retStr = "Abc Efg Hij", "Converts the first letter of every word in string to uppercase:" & retStr)

    'retStr = StrConv("ABCDEVB¥ì¥¹¥­¥å©`", vbWide)
    'MsgBox retStr
    'TestUtilModule.AssertTrue(retStr = "£Á£Â£Ã£Ä£ÅVB¥ì¥¹¥­¥å©`", "Converts narrow (single-byte) characters in string to wide")

    'retStr = StrConv("£Á£Â£Ã£Ä£ÅVB¥ì¥¹¥­¥å©`", vbNarrow)
    'MsgBox retStr
    'TestUtilModule.AssertTrue(retStr = "ABCDEVB¥ì¥¹¥­¥å©`", "Converts wide (double-byte) characters in string to narrow (single-byte) characters." & retStr)

    'retStr = StrConv("¤Ï¤Ê¤Á¤ã¤ó", vbKatakana)
    'MsgBox retStr
    'TestUtilModule.AssertTrue(retStr = "¥Ï¥Ê¥Á¥ã¥ó", "Converts Hiragana characters in string to Katakana characters.." & retStr)

   ' retStr = StrConv("¥Ï¥Ê¥Á¥ã¥ó", vbHiragana)
    'MsgBox retStr
   ' TestUtilModule.AssertTrue(retStr = "¤Ï¤Ê¤Á¤ã¤ó", "Converts Katakana characters in string to Hiragana characters.." & retStr)

    'x = StrConv("ÉÏº£ÊÐABC", vbFromUnicode)
    'MsgBox retStr
    'TestUtilModule.AssertTrue(UBound(x) = 8, "Converts the string from Unicode, the length is : " & UBound(x) + 1)

   ' retStr = StrConv(x, vbUnicode)
    'MsgBox retStr
   ' TestUtilModule.AssertTrue(retStr = "ÉÏº£ÊÐABC", "Converts the string to Unicode: " & retStr)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

