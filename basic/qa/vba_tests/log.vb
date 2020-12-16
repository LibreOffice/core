Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testLog()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testLog() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test Log function"
    On Error GoTo errorHandler

    date2 = 4.454
    date1 = Log(86)
    TestLog_ASSERT Round(date1, 3) = Round(date2, 3), "the return Log is: " & date1

    date2 = 4
    date1 = Exp(Log(4))
    TestLog_ASSERT date1 = date2, "the return Log is: " & date1

    date2 = 1
    date1 = Log(2.7182818)
    TestLog_ASSERT Round(date1, 3) = Round(date2, 3), "the return Log is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testLog = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


