Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testInt()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testInt() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test Int function"
    On Error GoTo errorHandler

    date2 = 99
    date1 = Int(99.8)
    TestLog_ASSERT date1 = date2, "the return Int is: " & date1

    date2 = -100
    date1 = Int(-99.8)
    TestLog_ASSERT date1 = date2, "the return Int is: " & date1

    date2 = -100
    date1 = Int(-99.2)
    TestLog_ASSERT date1 = date2, "the return Int is: " & date1

    date2 = 0
    date1 = Int(0.2)
    TestLog_ASSERT date1 = date2, "the return Int is: " & date1

    date2 = 0
    date1 = Int(0)
    TestLog_ASSERT date1 = date2, "the return Int is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testInt = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


