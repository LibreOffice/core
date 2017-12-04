Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testSTR()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testSTR() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test STR function"
    On Error GoTo errorHandler

    date2 = " 459"
    date1 = Str(459)
    TestLog_ASSERT date1 = date2, "the return STR is: " & date1

    date2 = "-459.65"
    date1 = Str(-459.65)
    TestLog_ASSERT date1 = date2, "the return STR is: " & date1

    date2 = " 459.001"
    date1 = Str(459.001)
    TestLog_ASSERT date1 = date2, "the return STR is: " & date1

    date2 = " .24"
    date1 = Str(0.24)
    TestLog_ASSERT date1 = date2, "the return STR is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testSTR = result


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

