Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testMinute()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testMinute() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test Minute function"
    On Error GoTo errorHandler

    date2 = 34
    date1 = Minute("09:34:20")
    TestLog_ASSERT date1 = date2, "the return Minute is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testMinute = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb

