Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testCInt()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCInt() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim nr1, nr2 As Integer   'variables for test
    testName = "Test CInt function"


    On Error GoTo errorHandler

    nr2 = -1
    nr1 = CInt(-1.1)
    TestLog_ASSERT nr1 = nr2, "the return CInt is: " & nr1

    nr2 = -1
    nr1 = CInt(-1.1)
    TestLog_ASSERT nr1 = nr2, "the return CInt is: " & nr1

    nr2 = -2
    nr1 = CInt(-1.9)
    TestLog_ASSERT nr1 = nr2, "the return CInt is: " & nr1

    nr2 = 0
    nr1 = CInt(0.2)
    TestLog_ASSERT nr1 = nr2, "the return CInt is: " & nr1

REM In excel:
REM    If the fraction is less than or equal to .5, the result will round down.
REM    If the fraction is greater than .5, the result will round up.

REM    nr2 = 0
REM    nr1 = CInt(0.5)
REM    TestLog_ASSERT nr1 = nr2, "the return CInt is: " & nr1

REM    nr2 = 2
REM    nr1 = CInt(1.5)
REM    TestLog_ASSERT nr1 = nr2, "the return CInt is: " & nr1

REM    nr2 = 2
REM    nr1 = CInt(2.5)
REM    TestLog_ASSERT nr1 = nr2, "the return CInt is: " & nr1

    nr2 = 11
    nr1 = CInt(10.51)
    TestLog_ASSERT nr1 = nr2, "the return CInt is: " & nr1

    nr2 = 30207
    nr1 = CInt("&H75FF")
    TestLog_ASSERT nr1 = nr2, "the return CInt is: " & nr1

    nr2 = 1876
    nr1 = CInt("&H754")
    TestLog_ASSERT nr1 = nr2, "the return CInt is: " & nr1

    nr2 = 21
    nr1 = CInt("+21")
    TestLog_ASSERT nr1 = nr2, "the return CInt is:" & nr1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCInt = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function



