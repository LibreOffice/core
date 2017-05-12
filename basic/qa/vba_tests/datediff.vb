Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testDateDiff()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testDateDiff() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim date1
    Dim date2
    testName = "Test DateDiff function"
    On Error GoTo errorHandler

    date2 = 10
    date1 = DateDiff("yyyy", "22/11/2003", "22/11/2013")
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 40
    date1 = DateDiff("q", "22/11/2003", "22/11/2013")
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 120
    date1 = DateDiff("m", "22/11/2003", "22/11/2013")
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3653
    date1 = DateDiff("y", "22/11/2003", "22/11/2013")
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013")
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 521
    date1 = DateDiff("w", "22/11/2003", "22/11/2013")
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 522
    date1 = DateDiff("ww", "22/11/2003", "22/11/2013")
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 87672
    date1 = DateDiff("h", "22/11/2003", "22/11/2013")
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 525600
    date1 = DateDiff("n", "22/11/2012", "22/11/2013")
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 2678400
    date1 = DateDiff("s", "22/10/2013", "22/11/2013")
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbFriday)
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbMonday)
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3623
    date1 = DateDiff("d", "22/12/2003", "22/11/2013", vbSaturday)
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3684
    date1 = DateDiff("d", "22/10/2003", "22/11/2013", vbSunday)
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbThursday)
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbTuesday)
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbFriday, vbFirstJan1)
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbThursday, vbFirstFourDays)
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbSunday, vbFirstFullWeek)
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbSaturday, vbFirstFullWeek)
    TestLog_ASSERT date1 = date2, "the return DateDiff is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testDateDiff = result

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

