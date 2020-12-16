Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testString()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testString() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestStr As String
    Dim date1, date2
    testName = "Test String function"
    On Error GoTo errorHandler

    date2 = "PPPPP"
    date1 = String(5, "P")
    TestLog_ASSERT date1 = date2, "the return String is: " & date1

    date2 = "aaaaa"
    date1 = String(5, "a")
    TestLog_ASSERT date1 = date2, "the return String is: " & date1

    date2 = ""
    date1 = String(0, "P")
    TestLog_ASSERT date1 = date2, "the return String is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testString = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


