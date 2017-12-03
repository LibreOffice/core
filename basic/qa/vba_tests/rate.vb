Option VBASupport 1
Rem Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testRATE()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testRATE() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim date1, date2
    testName = "Test RATE function"
    On Error GoTo errorHandler

    date2 = 0.07
    date1 = Rate(3, -5, 0, 16)
    TestLog_ASSERT Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1

    date2 = 0
    date1 = Rate(3, -5, 0, 15)
    TestLog_ASSERT Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1
    
    date2 = 0.79
    date1 = Rate(3, -5, 0, 30)
    TestLog_ASSERT Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1

    date2 = 1
    date1 = Rate(3, -5, 0, 35)
    TestLog_ASSERT Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1

    date2 = 0.077
    date1 = Rate(4, -300, 1000, 0, 0)
    TestLog_ASSERT Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1

    date2 = 0.14
    date1 = Rate(4, -300, 1000, 0, 1)
    TestLog_ASSERT Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testRATE = result

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

