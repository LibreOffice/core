Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testStrReverse()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testStrReverse() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test StrReverse function"
    On Error GoTo errorHandler

    date2 = "dcba"
    date1 = StrReverse("abcd")
    TestLog_ASSERT date1 = date2, "the return StrReverse is: " & date1

    date2 = "BABABA"
    date1 = StrReverse("ABABAB")
    TestLog_ASSERT date1 = date2, "the return StrReverse is: " & date1

    date2 = "654321"
    date1 = StrReverse("123456")
    TestLog_ASSERT date1 = date2, "the return StrReverse is: " & date1

    date2 = "6"
    date1 = StrReverse(6)
    TestLog_ASSERT date1 = date2, "the return StrReverse is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testStrReverse = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


