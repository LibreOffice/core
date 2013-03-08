Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testDateValue()
If failCount <> 0 And passCount > 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testDateValue() as String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim date1, date2 As Date
    testName = "Test DateValue function"
    date2 = 25246
    
    On Error GoTo errorHandler
    
    date1 = DateValue("February 12, 1969") '2/12/1969
    TestLog_ASSERT date1 = date2, "the return date is: " & date1
    
    date2 = 39468
    date1 = DateValue("21/01/2008") '1/21/2008
    TestLog_ASSERT date1 = date2, "the return date is: " & date1
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testDateValue = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False),  testName & ": hit error handler"
End Sub

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
