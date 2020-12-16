Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_SGN()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_SGN() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test SGN function"
    On Error GoTo errorHandler

    date2 = 0
    date1 = sgn(0)
    TestLog_ASSERT date1 = date2, "the return SGN is: " & date1

    date2 = -1
    date1 = sgn(-1)
    TestLog_ASSERT date1 = date2, "the return SGN is: " & date1

    date2 = 1
    date1 = sgn(1)
    TestLog_ASSERT date1 = date2, "the return SGN is: " & date1

    date2 = 1
    date1 = sgn(50)
    TestLog_ASSERT date1 = date2, "the return SGN is: " & date1

    date2 = -1
    date1 = sgn(-50)
    TestLog_ASSERT date1 = date2, "the return SGN is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_SGN = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


