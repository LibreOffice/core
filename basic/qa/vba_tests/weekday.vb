Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testWeekDay()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testWeekDay() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test WeekDay function"
    On Error GoTo errorHandler

    date2 = 7
    date1 = Weekday(#6/7/2009#, vbMonday)
    TestLog_ASSERT date1 = date2, "the return WeekDay is: " & date1

    date2 = 2
    date1 = Weekday(#7/7/2009#, vbMonday)
    TestLog_ASSERT date1 = date2, "the return WeekDay is: " & date1

    date2 = 5
    date1 = Weekday(#8/7/2009#, vbMonday)
    TestLog_ASSERT date1 = date2, "the return WeekDay is: " & date1

    date2 = 1
    date1 = Weekday(#12/7/2009#, vbMonday)
    TestLog_ASSERT date1 = date2, "the return WeekDay is: " & date1

    date2 = 1
    date1 = Weekday(#6/7/2009#, vbSunday)
    TestLog_ASSERT date1 = date2, "the return WeekDay is: " & date1

    date2 = 5
    date1 = Weekday(#6/7/2009#, 4)
    TestLog_ASSERT date1 = date2, "the return WeekDay is: " & date1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testWeekDay = result

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

