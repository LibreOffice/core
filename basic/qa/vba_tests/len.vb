Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testLen()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testLen() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test Len function"
    On Error GoTo errorHandler

    date2 = 8
    date1 = Len("sometext")
    TestLog_ASSERT date1 = date2, "the return Len is: " & date1

    date2 = 9
    date1 = Len("some text")
    TestLog_ASSERT date1 = date2, "the return Len is: " & date1

    date2 = 0
    date1 = Len("")
    TestLog_ASSERT date1 = date2, "the return Len is: " & date1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testLen = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


