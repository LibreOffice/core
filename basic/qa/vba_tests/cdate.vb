Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testCDate()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_testCDate() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2 As Date   'variables for test
    testName = "Test CDate function"
    On Error GoTo errorHandler

    date2 = 25246
    date1 = CDate("12/02/1969") '02/12/1969
    TestLog_ASSERT date1 = date2, "the return CDate is: " & date1

    date2 = 28313
    date1 = CDate("07/07/1977")
    TestLog_ASSERT date1 = date2, "the return CDate is: " & date1

    date2 = 28313
    date1 = CDate(#7/7/1977#)
    TestLog_ASSERT date1 = date2, "the return CDate is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCDate = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

