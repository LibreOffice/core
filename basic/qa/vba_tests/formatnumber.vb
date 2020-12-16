Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testFormatNumber
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testFormatNumber()

    TestUtilModule.TestInit

    Dim testName As String
    Dim str1 As String, str2 As String
    On Error GoTo errorHandler

    testName = "Test 1: positive, 2 decimals"
    str2 = "12.20"
    str1 = FormatNumber("12.2", 2, vbFalse, vbFalse, vbFalse)
    TestUtilModule.AssertTrue(str1 = str2, testName, "FormatNumber returned: " & str1)

    testName = "Test 2: negative, 20 decimals, use leading zero"
    str2 = "-0.20000000000000000000"
    str1 = FormatNumber("-.2", 20, vbTrue, vbFalse, vbFalse)
    TestUtilModule.AssertTrue(str1 = str2, testName, "FormatNumber returned: " & str1)

    testName = "Test 3: negative, 20 decimals, no leading zero"
    str2 = "-.20000000000000000000"
    str1 = FormatNumber("-0.2", 20, vbFalse, vbFalse, vbFalse)
    TestUtilModule.AssertTrue(str1 = str2, testName, "FormatNumber returned: " & str1)

    testName = "Test 4: negative, no leading zero, use parens"
    str2 = "(.20)"
    str1 = FormatNumber("-0.2", -1, vbFalse, vbTrue, vbFalse)
    TestUtilModule.AssertTrue(str1 = str2, testName, "FormatNumber returned: " & str1)

    testName = "Test 5: negative, default leading zero, use parens"
    str2 = "(0.20)"
    str1 = FormatNumber("-0.2", -1, vbUseDefault, vbTrue, vbFalse)
    TestUtilModule.AssertTrue(str1 = str2, testName, "FormatNumber returned: " & str1)

    testName = "Test 6: group digits"
    str2 = "-12,345,678.00"
    str1 = FormatNumber("-12345678", -1, vbUseDefault, vbUseDefault, vbTrue)
    TestUtilModule.AssertTrue(str1 = str2, testName, "FormatNumber returned: " & str1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
    TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

