Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testSQR()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testSQR() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test SQR function"
    On Error GoTo errorHandler

    date2 = 3
    date1 = Sqr(9)
    TestLog_ASSERT date1 = date2, "the return SQR is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testSQR = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb

