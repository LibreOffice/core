Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testMonth()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testMonth() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    Dim ldate As Date
    testName = "Test Month function"
    On Error GoTo errorHandler
    ldate = 32616

    date2 = 4
    date1 = Month(ldate)
    TestLog_ASSERT date1 = date2, "the return Month is: " & date1

    date2 = 2
    date1 = Month("01/02/2007")
    TestLog_ASSERT date1 = date2, "the return Month is: " & date1

    date2 = 12
    date1 = Month(1)
    TestLog_ASSERT date1 = date2, "the return Month is: " & date1

    date2 = 2
    date1 = Month(60)
    TestLog_ASSERT date1 = date2, "the return Month is: " & date1

    date2 = 6
    date1 = Month(2000)
    TestLog_ASSERT date1 = date2, "the return Month is: " & date1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testMonth = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


