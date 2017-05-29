Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testNPV()
If failCount <> 0 And passCount > 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testNPV() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test NPV function"
    On Error GoTo errorHandler
    Static Values(5) As Double    ' Set up array.
    Values(0) = -70000    ' Business start-up costs.
    ' Positive cash flows reflecting income for four successive years.
    Values(1) = 22000: Values(2) = 25000
    Values(3) = 28000: Values(4) = 31000

    date2 = 19312.57
    date1 = NPV(0.0625, Values())     ' Calculate net present value.
    TestLog_ASSERT Round(date1, 2) = Round(date2, 2), "the return NPV is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testNPV = result

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

