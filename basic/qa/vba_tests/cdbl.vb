Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testCdbl()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCdbl() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim nr1, nr2 As Double   'variables for test
    testName = "Test Cdbl function"
    On Error GoTo errorHandler

    nr2 = 0
    nr1 = CDbl(0)
    TestLog_ASSERT nr1 = nr2, "the return Cdbl is: " & nr1

    nr2 = 10.1234567890123
    nr1 = CDbl(10.1234567890123)
    TestLog_ASSERT nr1 = nr2, "the return Cdbl is: " & nr1

    nr2 = 0.00005
    nr1 = CDbl(0.005 * 0.01)
    TestLog_ASSERT nr1 = nr2, "the return Cdbl is: " & nr1

    nr2 = 20
    nr1 = CDbl("20")
    TestLog_ASSERT nr1 = nr2, "the return Cdbl is: " & nr1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCdbl = result

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

