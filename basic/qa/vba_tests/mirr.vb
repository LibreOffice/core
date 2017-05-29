Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testMIRR()
If failCount <> 0 And passCount > 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testMIRR() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2, LoanAPR, InvAPR, Fmt, RetRate, Msg
    testName = "Test MIRR function"
    On Error GoTo errorHandler

    Static Values(5) As Double    ' Set up array.
    LoanAPR = 0.1   ' Loan rate.
    InvAPR = 0.12   ' Reinvestment rate.
    Values(0) = -70000    ' Business start-up costs.
    Values(1) = 22000: Values(2) = 25000
    Values(3) = 28000: Values(4) = 31000
    date2 = 0.148
    date1 = MIRR(Values(), LoanAPR, InvAPR)
    TestLog_ASSERT Round(date1, 3) = Round(date2, 3), "the return MIRR is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testMIRR = result

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

