Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testSTR()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testSTR() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test STR function"
    On Error GoTo errorHandler

    date2 = " 459"
    date1 = Str(459)
    TestLog_ASSERT date1 = date2, "the return STR is: " & date1

    date2 = "-459.65"
    date1 = Str(-459.65)
    TestLog_ASSERT date1 = date2, "the return STR is: " & date1

    date2 = " 459.001"
    date1 = Str(459.001)
    TestLog_ASSERT date1 = date2, "the return STR is: " & date1

    date2 = " .24"
    date1 = Str(0.24)
    TestLog_ASSERT date1 = date2, "the return STR is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testSTR = result


    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb

