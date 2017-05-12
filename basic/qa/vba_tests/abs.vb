Rem Attribute VBA_ModuleType=VBAModule
Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testABS()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testABS() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim date1, date2 As Date   'variables for test
    Dim nr1, nr2 As Double

    testName = "Test ABS function"
    On Error GoTo errorHandler

    nr2 = 5
    nr1 = Abs(-5)
    TestLog_ASSERT nr1 = nr2, "the return ABS is: " & nr1

    nr2 = 5
    nr1 = Abs(5)
    TestLog_ASSERT nr1 = nr2, "the return ABS is: " & nr1

    nr2 = 21.7
    nr1 = Abs(-21.7)
    TestLog_ASSERT nr1 = nr2, "the return ABS is: " & nr1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testABS = result

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

