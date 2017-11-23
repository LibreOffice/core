Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testDateAdd()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testDateAdd() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2 As Date   'variables for test
    testName = "Test DateAdd function"
    On Error GoTo errorHandler

    date2 = CDate("1995-02-28")
    date1 = DateAdd("m", 1, "1995-01-31")
    TestLog_ASSERT date1 = date2, "the return DateAdd is: " & date1

    date2 = CDate("1995-02-28")
    date1 = DateAdd("m", 1, "1995-01-31")
    TestLog_ASSERT date1 = date2, "the return DateAdd is: " & date1

    date2 = CDate("1995-02-28")
    date1 = DateAdd("m", 1, "1995-01-31")
    TestLog_ASSERT date1 = date2, "the return DateAdd is: " & date1

    date2 = CDate("1996-01-31")
    date1 = DateAdd("yyyy", 1, "1995-01-31")
    TestLog_ASSERT date1 = date2, "the return DateAdd is: " & date1

    date2 = CDate("1995-04-30")
    date1 = DateAdd("q", 1, "1995-01-31")
    TestLog_ASSERT date1 = date2, "the return DateAdd is: " & date1

    date2 = CDate("1995-02-01")
    date1 = DateAdd("y", 1, "1995-01-31")
    TestLog_ASSERT date1 = date2, "the return DateAdd is: " & date1

    date2 = CDate("1995-02-01")
    date1 = DateAdd("d", 1, "1995-01-31")
    TestLog_ASSERT date1 = date2, "the return DateAdd is: " & date1

    date2 = CDate("1995-02-01")
    date1 = DateAdd("w", 1, "1995-01-31")
    TestLog_ASSERT date1 = date2, "the return DateAdd is: " & date1

    date2 = CDate("1995-02-07")
    date1 = DateAdd("ww", 1, "1995-01-31")
    TestLog_ASSERT date1 = date2, "the return DateAdd is: " & date1

Rem This fails when directly comparing date1=date2, probably due to rounding.
Rem Workaround convert to string which does the rounding.
    Dim date1s, date2s As String
    date2 = CDate("1995-01-01 22:48:29")
    date1 = DateAdd("h", 1, "1995-01-01 21:48:29")
    date1s = "" & date1
    date2s = "" & date2
    TestLog_ASSERT date1s = date2s, "the return DateAdd is: " & date1

    date2 = CDate("1995-01-31 21:49:29")
    date1 = DateAdd("n", 1, "1995-01-31 21:48:29")
    TestLog_ASSERT date1 = date2, "the return DateAdd is: " & date1

    date2 = CDate("1995-01-31 21:48:30")
    date1 = DateAdd("s", 1, "1995-01-31 21:48:29")
    TestLog_ASSERT date1 = date2, "the return DateAdd is: " & date1

exitFunc:
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testDateAdd = result

    Exit Function

errorHandler:
    On Error GoTo 0
    TestLog_ASSERT (False), testName & ": hit error handler"
    GoTo exitFunc

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

