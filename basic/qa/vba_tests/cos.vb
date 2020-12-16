Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testCOS()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCOS() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim nr1, nr2 As Double   'variables for test
    testName = "Test COS function"


    On Error GoTo errorHandler

    nr2 = -0.532833020333398
    nr1 = Cos(23)
    TestLog_ASSERT Round(nr1, 14) = Round(nr2, 14), "the return COS is: " & nr1

    nr2 = 0.980066577841242
    nr1 = Cos(0.2)
    TestLog_ASSERT Round(nr1, 14) = Round(nr2, 14), "the return COS is: " & nr1

    nr2 = 0.487187675007006
    nr1 = Cos(200)
    TestLog_ASSERT Round(nr1, 14) = Round(nr2, 14), "the return COS is: " & nr1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCOS = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


