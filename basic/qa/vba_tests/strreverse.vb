Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testStrReverse()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testStrReverse() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test StrReverse function"
    On Error GoTo errorHandler

    date2 = "dcba"
    date1 = StrReverse("abcd")
    TestLog_ASSERT date1 = date2, "the return StrReverse is: " & date1

    date2 = "BABABA"
    date1 = StrReverse("ABABAB")
    TestLog_ASSERT date1 = date2, "the return StrReverse is: " & date1

    date2 = "654321"
    date1 = StrReverse("123456")
    TestLog_ASSERT date1 = date2, "the return StrReverse is: " & date1

    date2 = "6"
    date1 = StrReverse(6)
    TestLog_ASSERT date1 = date2, "the return StrReverse is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testStrReverse = result

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

