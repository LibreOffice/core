Option VBASupport 1
Rem Option VBASupport 1    'unREM in .vb file
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testCBool()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCBool() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim res2, res1 As Boolean
    Dim a1, a2 As Integer
    testName = "Test CBool function"
    On Error GoTo errorHandler

    res2 = True
    res1 = CBool(1)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = False
    res1 = CBool(1 = 2)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = False
    res1 = CBool(0)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = True
    res1 = CBool(21)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = True
    res1 = CBool("true")
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = False
    res1 = CBool("false")
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = True
    res1 = CBool("1")
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = True
    res1 = CBool("-1")
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = False
    res1 = CBool("0")
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = False
    a1 = 1: a2 = 10
    res1 = CBool(a1 = a2)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = True
    a1 = 10: a2 = 10
    res1 = CBool(a1 = a2)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCBool = result

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

