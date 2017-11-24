Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testCOS()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCOS() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim nr1, nr2 As Double   'variables for test
    testName = "Test COS function"


    On Error GoTo errorHandler

    nr2 = -0.532833020333398
    nr1 = Cos(23)
    TestLog_ASSERT Round(nr1, 14) = Round(nr2, 14), "the return COS is: " & nr1

    nr2 = 0.980066577841242
    nr1 = Cos(0.2)
    TestLog_ASSERT Round(nr1, 14) = Round(nr2, 14), "the return COS is: " & nr1

    nr2 = 0.487187675007006
    nr1 = Cos(200)
    TestLog_ASSERT Round(nr1, 14) = Round(nr2, 14), "the return COS is: " & nr1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCOS = result

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

