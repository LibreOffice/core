Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testCVErr()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCVErr() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim date1, date2
    testName = "Test CVErr function"
    On Error GoTo errorHandler

    date2 = "Error 3001"
    date1 = CStr(CVErr(3001))
    TestLog_ASSERT date1 = date2, "the return CVErr is: " & date1

    date2 = "Error 2007"
    date1 = CStr(CVErr(xlErrDiv0))
    TestLog_ASSERT date1 = date2, "the return CVErr is: " & date1

    date2 = "Error 2042"
    date1 = CStr(CVErr(xlErrNA))
    TestLog_ASSERT date1 = date2, "the return CVErr is: " & date1

    date2 = "Error 2029"
    date1 = CStr(CVErr(xlErrName))
    TestLog_ASSERT date1 = date2, "the return CVErr is: " & date1

    date2 = "Error 2000"
    date1 = CStr(CVErr(xlErrNull))
    TestLog_ASSERT date1 = date2, "the return CVErr is: " & date1

    date2 = "Error 2036"
    date1 = CStr(CVErr(xlErrNum))
    TestLog_ASSERT date1 = date2, "the return CVErr is: " & date1

    date2 = "Error 2023"
    date1 = CStr(CVErr(xlErrRef))
    TestLog_ASSERT date1 = date2, "the return CVErr is: " & date1

    date2 = "Error 2015"
    date1 = CStr(CVErr(xlErrValue))
    TestLog_ASSERT date1 = date2, "the return CVErr is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCVErr = result

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

