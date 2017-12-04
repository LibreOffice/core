Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testWeekDayName()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testWeekDayName() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test WeekDayName function"
    On Error GoTo errorHandler

    date2 = "Sunday"
    date1 = WeekdayName(1)
    TestLog_ASSERT date1 = date2, "the return WeekDayName is: " & date1

    date2 = "Sunday"
    date1 = WeekdayName(1, , vbSunday)
    TestLog_ASSERT date1 = date2, "the return WeekDayName is: " & date1

    date2 = "Monday"
    date1 = WeekdayName(1, , vbMonday)
    TestLog_ASSERT date1 = date2, "the return WeekDayName is: " & date1

    date2 = "Monday"
    date1 = WeekdayName(2)
    TestLog_ASSERT date1 = date2, "the return WeekDayName is: " & date1

    date2 = "Tue"
    date1 = WeekdayName(2, True, vbMonday)
    TestLog_ASSERT date1 = date2, "the return WeekDayName is: " & date1

    date2 = "Wed"
    date1 = WeekdayName(2, True, vbTuesday)
    TestLog_ASSERT date1 = date2, "the return WeekDayName is: " & date1

    date2 = "Thu"
    date1 = WeekdayName(2, True, vbWednesday)
    TestLog_ASSERT date1 = date2, "the return WeekDayName is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testWeekDayName = result

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

