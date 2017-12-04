Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testVarType()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testVarType() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim TestInt As Integer
    Dim TestLong As Long
    Dim TestDouble As Double
    Dim TestBoo As Boolean
    Dim date1, date2
    testName = "Test VarType function"
    On Error GoTo errorHandler

    date2 = 8
    date1 = VarType(testName)
    TestLog_ASSERT date1 = date2, "the return VarType is: " & date1

    date2 = 11
    date1 = VarType(TestBoo)
    TestLog_ASSERT date1 = date2, "the return VarType is: " & date1

    date2 = 5
    date1 = VarType(TestDouble)
    TestLog_ASSERT date1 = date2, "the return VarType is: " & date1

    date2 = 3
    date1 = VarType(TestLong)
    TestLog_ASSERT date1 = date2, "the return VarType is: " & date1

    date2 = 2
    date1 = VarType(TestInt)
    TestLog_ASSERT date1 = date2, "the return VarType is: " & date1

    date2 = 7
    date1 = VarType(TestDateTime)
    TestLog_ASSERT date1 = date2, "the return VarType is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testVarType = result

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

