Option VBASupport 1
Option Explicit

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testDateSerial()
If failCount <> 0 And passCount > 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_testDateSerial() as String
    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim date1, date2 As Date
    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    testName = "Test DateSerial function"
    date2 = 36326
    
    On Error GoTo errorHandler
    
    date1 = DateSerial(1999, 6, 15)   '6/15/1999
    TestLog_ASSERT date1 = date2, "the return date is: " & date1
    date1 = DateSerial(2000, 1 - 7, 15) '6/15/1999
    TestLog_ASSERT date1 = date2, "the return date is: " & date1
    date1 = DateSerial(1999, 1, 166)  '6/15/1999
    TestLog_ASSERT date1 = date2, "the return date is: " & date1
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    
    verify_testDateSerial = result

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

