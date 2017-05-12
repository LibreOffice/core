Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testChoose()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testChoose() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim var1, var2
    testName = "Test Choose function"


    On Error GoTo errorHandler

    var2 = "Libre"
    var1 = Choose(1, "Libre", "Office", "Suite")
    TestLog_ASSERT var1 = var2, "the return Choose is: " & var1

    var2 = "Office"
    var1 = Choose(2, "Libre", "Office", "Suite")
    TestLog_ASSERT var1 = var2, "the return Choose is: " & var1

    var2 = "Suite"
    var1 = Choose(3, "Libre", "Office", "Suite")
    TestLog_ASSERT var1 = var2, "the return Choose is: " & var1


    var1 = Choose(4, "Libre", "Office", "Suite")
    TestLog_ASSERT IsNull(var1), "the return Choose is: Null4 "  

    var1 = Choose(0, "Libre", "Office", "Suite")
    TestLog_ASSERT IsNull(var1), "the return Choose is: Null0 " 

    var1 = Choose(-1, "Libre", "Office", "Suite")
    TestLog_ASSERT IsNull(var1), "the return Choose is: Null-1"

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testChoose = result

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

