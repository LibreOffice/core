Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testIsNumeric()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testIsNumeric() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test IsNumeric function"
    On Error GoTo errorHandler

    date2 = True
    date1 = IsNumeric(123)
    TestLog_ASSERT date1 = date2, "the return IsNumeric is: " & date1

    date2 = True
    date1 = IsNumeric(-123)
    TestLog_ASSERT date1 = date2, "the return IsNumeric is: " & date1

    date2 = True
    date1 = IsNumeric(123.8)
    TestLog_ASSERT date1 = date2, "the return IsNumeric is: " & date1

    date2 = False
    date1 = IsNumeric("a")
    TestLog_ASSERT date1 = date2, "the return IsNumeric is: " & date1

rem    date2 = True
rem    date1 = IsNumeric(True)
rem    TestLog_ASSERT date1 = date2, "the return IsNumeric is: " & date1

    date2 = True
    date1 = IsNumeric("123")
    TestLog_ASSERT date1 = date2, "the return IsNumeric is: " & date1

    date2 = True
    date1 = IsNumeric("+123")
    TestLog_ASSERT date1 = date2, "the return IsNumeric is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testIsNumeric = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


