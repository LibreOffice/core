Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testPMT()
If failCount <> 0 And passCount > 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testPMT() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test PMT function"
    On Error GoTo errorHandler

    date2 = -255.14
    date1 = Pmt(0.0083, 48, 10000, 100, 0)
    TestLog_ASSERT Round(date1, 2) = Round(date2, 2), "the return PMT is: " & date1

    date2 = -253.04
    date1 = Pmt(0.0083, 48, 10000, 100, 1)
    TestLog_ASSERT Round(date1, 2) = Round(date2, 2), "the return PMT is: " & date1

    date2 = -253.43
    date1 = Pmt(0.0083, 48, 10000)
    TestLog_ASSERT Round(date1, 2) = Round(date2, 2), "the return PMT is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testPMT = result

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

