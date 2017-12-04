Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testVal()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testVal() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test Val function"
    On Error GoTo errorHandler

    date2 = 2
    date1 = Val("02/04/2010")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 1050
    date1 = Val("1050")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 130.75
    date1 = Val("130.75")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 50
    date1 = Val("50 Park Lane")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 1320
    date1 = Val("1320 then some text")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 0
    date1 = Val("L13.5")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 0
    date1 = Val("sometext")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

REM    date2 = 1
REM    date1 = Val("1, 2")
REM    TestLog_ASSERT date1 = date2, "the return Val is: " & date1
REM		tdf#111999

    date2 = -1
    date1 = Val("&HFFFF")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testVal = result

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

