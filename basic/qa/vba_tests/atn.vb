Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testATN()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testATN() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim nr1, nr2 As Double
    testName = "Test ATN function"
    On Error GoTo errorHandler

    nr2 = 1.10714871779409
    nr1 = Atn(2)
    TestLog_ASSERT Round(nr1, 14) = Round(nr2, 14), "the return ATN is: " & nr1

    nr2 = 1.19166451926354
    nr1 = Atn(2.51)
    TestLog_ASSERT Round(nr1, 14) = Round(nr2, 14), "the return ATN is: " & nr1

    nr2 = -1.27229739520872
    nr1 = Atn(-3.25)
    TestLog_ASSERT Round(nr1, 14) = Round(nr2, 14), "the return ATN is: " & nr1

    nr2 = 1.56603445802574
    nr1 = Atn(210)
    TestLog_ASSERT Round(nr1, 14) = Round(nr2, 14), "the return ATN is: " & nr1

    nr2 = 0
    nr1 = Atn(0)
    TestLog_ASSERT nr1 = nr2, "the return ATN is: " & nr1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testATN = result

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


