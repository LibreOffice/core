Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testCDec()
If failCount <> 0 And passCount > 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_testCDec() as String
    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim ret As Double
    testName = "Test CDec function"
    On Error GoTo errorHandler
    
    ret = CDec("")
    TestLog_ASSERT ret = 0, "Converts the string to uppercase characters:" & ret

    ret = CDec("1234")
    TestLog_ASSERT ret = "1234", "Converts the string to uppercase characters:" & ret

    ret = CDec("  1234  ")
    TestLog_ASSERT ret = 1234, "Converts the string to uppercase characters:" & ret

    '''''''''''''''
    ' Those are erroneous, see i#64348
    ret = CDec("1234-")
    TestLog_ASSERT ret = -1234, "Converts the string to uppercase characters:" & ret

    ret = CDec("  1234  -")
    TestLog_ASSERT ret = -1234, "Converts the string to uppercase characters:" & ret

    ret = CDec("79228162514264400000000000000")
    TestLog_ASSERT ret = 62406456049664, "Converts the string to uppercase characters:" & ret

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCDec = result

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
