Option VBASupport 1
Option Explicit

#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testDateSerial()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_testDateSerial() as String
    Dim testName As String
    Dim date1, date2 As Date
    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    testName = "Test DateSerial function"
    date2 = 36326

    On Error GoTo errorHandler

    date1 = DateSerial(1999, 6, 15)   '6/15/1999
    TestLog_ASSERT date1 = date2, "the return date is: " & date1
    date1 = DateSerial(2000, 1 - 7, 15) '6/15/1999
    TestLog_ASSERT date1 = date2, "the return date is: " & date1
    date1 = DateSerial(1999, 1, 166)  '6/15/1999
    TestLog_ASSERT date1 = date2, "the return date is: " & date1
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)

    verify_testDateSerial = result

    Exit Function
errorHandler:
    TestLog_ASSERT (False), testName & ": hit error handler"
End Function


