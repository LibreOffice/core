Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testOct()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testOct() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test Oct function"
    On Error GoTo errorHandler

    date2 = 4
    date1 = Oct(4)
    TestLog_ASSERT date1 = date2, "the return Oct is: " & date1

    date2 = 10
    date1 = Oct(8)
    TestLog_ASSERT date1 = date2, "the return Oct is: " & date1

    date2 = 713
    date1 = Oct(459)
    TestLog_ASSERT date1 = date2, "the return Oct is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testOct = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


