Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testCHR()
If failCount <> 0 And passCount > 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCHR() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim str1, str2 As String
    testName = "Test CHR function"
    On Error GoTo errorHandler
    
    str2 = "W"
    str1 = Chr(87)
    TestLog_ASSERT str1 = str2, "the return CHR is: " & str1
    
    str2 = "i"
    str1 = Chr(105)
    TestLog_ASSERT str1 = str2, "the return CHR is: " & str1
    
    str2 = "#"
    str1 = Chr(35)
    TestLog_ASSERT str1 = str2, "the return CHR is: " & str1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCHR = result

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


