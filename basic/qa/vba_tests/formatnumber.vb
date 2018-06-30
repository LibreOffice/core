Option VBASupport 1
Option Explicit

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testFormatNumber()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_testFormatNumber() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim str1 As String, str2 As String
    On Error GoTo errorHandler

    testName = "Test 1: positive, 2 decimals"
    str2 = "12.20"
    str1 = FormatNumber("12.2", 2, vbFalse, vbFalse, vbFalse)
    TestLog_ASSERT str1 = str2, testName, "FormatNumber returned: " & str1

    testName = "Test 2: negative, 20 decimals, use leading zero"
    str2 = "-0.20000000000000000000"
    str1 = FormatNumber("-.2", 20, vbTrue, vbFalse, vbFalse)
    TestLog_ASSERT str1 = str2, testName, "FormatNumber returned: " & str1

    testName = "Test 3: negative, 20 decimals, no leading zero"
    str2 = "-.20000000000000000000"
    str1 = FormatNumber("-0.2", 20, vbFalse, vbFalse, vbFalse)
    TestLog_ASSERT str1 = str2, testName, "FormatNumber returned: " & str1

    testName = "Test 4: negative, no leading zero, use parens"
    str2 = "(.20)"
    str1 = FormatNumber("-0.2", -1, vbFalse, vbTrue, vbFalse)
    TestLog_ASSERT str1 = str2, testName, "FormatNumber returned: " & str1

    testName = "Test 5: negative, default leading zero, use parens"
    str2 = "(0.20)"
    str1 = FormatNumber("-0.2", -1, vbUseDefault, vbTrue, vbFalse)
    TestLog_ASSERT str1 = str2, testName, "FormatNumber returned: " & str1

    testName = "Test 6: group digits"
    str2 = "-12,345,678.00"
    str1 = FormatNumber("-12345678", -1, vbUseDefault, vbUseDefault, vbTrue)
    TestLog_ASSERT str1 = str2, testName, "FormatNumber returned: " & str1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testFormatNumber = result

    Exit Function
errorHandler:
    TestLog_ASSERT False, testName & ": hit error handler"
End Function

Sub TestLog_ASSERT(assertion As Boolean, Optional testId As String, Optional testComment As String)

    If assertion = True Then
        passCount = passCount + 1
    Else
        Dim testMsg As String
        If Not IsMissing(testId) Then
            testMsg = testId & " "
        End If
        If Not IsMissing(testComment) And Not (testComment = "") Then
            testMsg = testMsg + "(" + testComment + ")"
        End If

        result = result & Chr$(10) & " Failed: " & testMsg
        failCount = failCount + 1
    End If

End Sub
