Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testCSng()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCSng() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim nr1, nr2 As Single   'variables for test
    Dim nr3 As Double

    testName = "Test CSng function"
    On Error GoTo errorHandler

    nr2 = 8.534535408
    nr1 = CSng(8.534535408)
    TestLog_ASSERT nr1 = nr2, "the return CSng is: " & nr1

    nr3 = 100.1234
    nr2 = 100.1234
    nr1 = CSng(nr3)
    TestLog_ASSERT nr1 = nr2, "the return CSng is: " & nr1

    nr2 = 0
    nr1 = CSng(0)
    TestLog_ASSERT nr1 = nr2, "the return CSng is: " & nr1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCSng = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


