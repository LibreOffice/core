Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testTimeValue()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_testTimeValue() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String

    Dim date1, date2 As Date   'variables for test
    testName = "Test TimeValue function"
    On Error GoTo errorHandler

    date2 = "16:35:17"
    date1 = TimeValue("4:35:17 PM")
    TestLog_ASSERT date1 = date2, "the return TimeValue is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testTimeValue = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


