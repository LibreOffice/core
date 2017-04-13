Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testLCase()
If failCount <> 0 And passCount > 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testLCase() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim date1, date2 As Date   'variables for test
    testName = "Test LCase function"
    On Error GoTo errorHandler

    date2 = "lowercase"
    date1 = LCase("LOWERCASE")
    TestLog_ASSERT date1 = date2, "the return LCase is: " & date1

    date2 = "lowercase"
    date1 = LCase("LowerCase")
    TestLog_ASSERT date1 = date2, "the return LCase is: " & date1

    date2 = "lowercase"
    date1 = LCase("lowercase")
    TestLog_ASSERT date1 = date2, "the return LCase is: " & date1

    date2 = "lowercase"
    date1 = LCase("LOWER CASE")
    TestLog_ASSERT date1 = date2, "the return LCase is: " & date1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testLCase = result

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

