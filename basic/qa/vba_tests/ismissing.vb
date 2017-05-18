Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testIsMissing()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testIsMissing() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim num1, num2 As Integer
    testName = "Test IsMissing function"
    On Error GoTo errorHandler

    num1 = ReturnTwice()
    TestLog_ASSERT IsNull(num1)

    num2 = 4
    num1 = ReturnTwice(2)
    TestLog_ASSERT num1 = num2, "the return IsMissing is: " & num1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testIsMissing = result

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
' Function procedure definition.
Function ReturnTwice(Optional A)
    If IsMissing(A) Then
        ' If argument is missing, return a Null.
        ReturnTwice = Null
    Else
        ' If argument is present, return twice the value.
        ReturnTwice = A * 2
    End If
End Function
