Rem Attribute VBA_ModuleType=VBAModule
Option VBASupport 1
Option Explicit

#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testABS()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function


Function verify_testABS() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim nr1, nr2 As Double

    testName = "Test ABS function"
    On Error GoTo errorHandler

    nr2 = 5
    nr1 = Abs(-5)
    TestLog_ASSERT nr1 = nr2, "the return ABS is: " & nr1

    nr2 = 5
    nr1 = Abs(5)
    TestLog_ASSERT nr1 = nr2, "the return ABS is: " & nr1

    nr2 = 21.7
    nr1 = Abs(-21.7)
    TestLog_ASSERT nr1 = nr2, "the return ABS is: " & nr1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testABS = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

