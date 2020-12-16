Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testCStr()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCStr() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim str2, str3
    Dim str1 As String   'variables for test
    testName = "Test CStr function"
    On Error GoTo errorHandler

    str3 = 437.324
    str2 = "437.324"
    str1 = CStr(str3)
    TestLog_ASSERT str1 = str2, "the return CStr is: " & str1

    str2 = "500"
    str1 = CStr(500)
    TestLog_ASSERT str1 = str2, "the return CStr is: " & str1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCStr = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb

