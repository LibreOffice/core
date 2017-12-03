Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_SGN()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_SGN() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test SGN function"
    On Error GoTo errorHandler

    date2 = 0
    date1 = sgn(0)
    TestLog_ASSERT date1 = date2, "the return SGN is: " & date1

    date2 = -1
    date1 = sgn(-1)
    TestLog_ASSERT date1 = date2, "the return SGN is: " & date1

    date2 = 1
    date1 = sgn(1)
    TestLog_ASSERT date1 = date2, "the return SGN is: " & date1

    date2 = 1
    date1 = sgn(50)
    TestLog_ASSERT date1 = date2, "the return SGN is: " & date1

    date2 = -1
    date1 = sgn(-50)
    TestLog_ASSERT date1 = date2, "the return SGN is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_SGN = result

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

