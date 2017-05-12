Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testCLng()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCLng() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim nr1, nr2 As Long   'variables for test
    testName = "Test CLng function"


    On Error GoTo errorHandler

    nr2 = -1
    nr1 = CLng(-1.1)
    TestLog_ASSERT nr1 = nr2, "the return CLng is: " & nr1

    nr2 = -1
    nr1 = CLng(-1.1)
    TestLog_ASSERT nr1 = nr2, "the return CLng is: " & nr1

    nr2 = -2
    nr1 = CLng(-1.9)
    TestLog_ASSERT nr1 = nr2, "the return CLng is: " & nr1

    nr2 = 0
    nr1 = CLng(0.2)
    TestLog_ASSERT nr1 = nr2, "the return CLng is: " & nr1

REM    nr2 = 0
REM    nr1 = CLng(0.5)
REM    TestLog_ASSERT nr1 = nr2, "the return CLng is: " & nr1

REM    If the fraction is less than or equal to .5, the result will round down.
REM    If the fraction is greater than .5, the result will round up.

    nr2 = 11
    nr1 = CLng(10.51)
    TestLog_ASSERT nr1 = nr2, "the return CLng is: " & nr1

    nr2 = 30207
    nr1 = CLng("&H75FF")
    TestLog_ASSERT nr1 = nr2, "the return CLng is: " & nr1

    nr2 = 1876
    nr1 = CLng("&H754")
    TestLog_ASSERT nr1 = nr2, "the return CLng is: " & nr1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCLng = result

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



