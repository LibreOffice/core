Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testMonthName()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testMonthName() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2 
    testName = "Test MonthName function"
    On Error GoTo errorHandler

    date2 = "February"
    date1 = MonthName(2)
    TestLog_ASSERT date1 = date2, "the return MonthName is: " & date1

    date2 = "Feb"
    date1 = MonthName(2, True)
    TestLog_ASSERT date1 = date2, "the return MonthName is: " & date1

    date2 = "February"
    date1 = MonthName(2, False)
    TestLog_ASSERT date1 = date2, "the return MonthName is: " & date1
    
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testMonthName = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


