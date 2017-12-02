Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testDatePart()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testDatePart() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1   'variables for test
    Dim date2
    testName = "Test DatePart function"
    On Error GoTo errorHandler

    date2 = 1969
    date1 = DatePart("yyyy", "1969-02-12")
    TestLog_ASSERT date1 = date2, "the return DatePart is: " & date1

    date2 = 1
    date1 = DatePart("q", "1969-02-12")
    TestLog_ASSERT date1 = date2, "the return DatePart is: " & date1

    date2 = 43
    date1 = DatePart("y", "1969-02-12")
    TestLog_ASSERT date1 = date2, "the return DatePart is: " & date1

    date2 = 12
    date1 = DatePart("d", "1969-02-12")
    TestLog_ASSERT date1 = date2, "the return DatePart is: " & date1

    date2 = 4
    date1 = DatePart("w", "1969-02-12")
    TestLog_ASSERT date1 = date2, "the return DatePart is: " & date1

    date2 = 7
    date1 = DatePart("ww", "1969-02-12")
    TestLog_ASSERT date1 = date2, "the return DatePart is: " & date1

    date2 = 16
    date1 = DatePart("h", "1969-02-12 16:32:00")
    TestLog_ASSERT date1 = date2, "the return DatePart is: " & date1

    date2 = 32
    date1 = DatePart("n", "1969-02-12 16:32:00")
    TestLog_ASSERT date1 = date2, "the return DatePart is: " & date1

    date2 = 0
    date1 = DatePart("s", "1969-02-12 16:32:00")
    TestLog_ASSERT date1 = date2, "the return DatePart is: " & date1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testDatePart = result

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

