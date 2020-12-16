Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testRight()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testRight() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test Right function"
    On Error GoTo errorHandler

    date2 = "text"
    date1 = Right("sometext", 4)
    TestLog_ASSERT date1 = date2, "the return Right is: " & date1

    date2 = "sometext"
    date1 = Right("sometext", 48)
    TestLog_ASSERT date1 = date2, "the return Right is: " & date1

    date2 = ""
    date1 = Right("", 4)
    TestLog_ASSERT date1 = date2, "the return Right is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testRight = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


