Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testTimeSerial()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testTimeSerial() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2 As Date
    testName = "Test TimeSerial function"
    On Error GoTo errorHandler

rem    bug 114229
rem    date2 = "5:45:00"
rem    date1 = (TimeSerial(12 - 6, -15, 0))
rem    TestLog_ASSERT date1 = date2, "the return TimeSerial is: " & date1

    date2 = "12:30:00"
    date1 = TimeSerial(12, 30, 0)
    TestLog_ASSERT date1 = date2, "the return TimeSerial is: " & date1

rem    date2 = "11:30:00"
rem    date1 = TimeSerial(10, 90, 0)
rem    TestLog_ASSERT date1 = date2, "the return TimeSerial is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testTimeSerial = result

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

