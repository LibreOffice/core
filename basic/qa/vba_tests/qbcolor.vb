Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testQBcolor()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testQBcolor() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2 As Long
    testName = "Test QBcolor function"
    On Error GoTo errorHandler

    date2 = 0
    date1 = QBColor(0)
    TestLog_ASSERT date1 = date2, "the return QBcolor is: " & date1

    date2 = 8388608
    date1 = QBColor(1)
    TestLog_ASSERT date1 = date2, "the return QBcolor is: " & date1

    date2 = 32768
    date1 = QBColor(2)
    TestLog_ASSERT date1 = date2, "the return QBcolor is: " & date1

    date2 = 8421376
    date1 = QBColor(3)
    TestLog_ASSERT date1 = date2, "the return QBcolor is: " & date1

    date2 = 128
    date1 = QBColor(4)
    TestLog_ASSERT date1 = date2, "the return QBcolor is: " & date1

    date2 = 8388736
    date1 = QBColor(5)
    TestLog_ASSERT date1 = date2, "the return QBcolor is: " & date1

    date2 = 32896
    date1 = QBColor(6)
    TestLog_ASSERT date1 = date2, "the return QBcolor is: " & date1

    date2 = 12632256
    date1 = QBColor(7)
    TestLog_ASSERT date1 = date2, "the return QBcolor is: " & date1

    date2 = 8421504
    date1 = QBColor(8)
    TestLog_ASSERT date1 = date2, "the return QBcolor is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testQBcolor = result

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

