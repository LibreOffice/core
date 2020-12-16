Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testCdbl()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCdbl() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim nr1, nr2 As Double   'variables for test
    testName = "Test Cdbl function"
    On Error GoTo errorHandler

    nr2 = 0
    nr1 = CDbl(0)
    TestLog_ASSERT nr1 = nr2, "the return Cdbl is: " & nr1

    nr2 = 10.1234567890123
    nr1 = CDbl(10.1234567890123)
    TestLog_ASSERT nr1 = nr2, "the return Cdbl is: " & nr1

    nr2 = 0.00005
    nr1 = CDbl(0.005 * 0.01)
    TestLog_ASSERT nr1 = nr2, "the return Cdbl is: " & nr1

    nr2 = 20
    nr1 = CDbl("20")
    TestLog_ASSERT nr1 = nr2, "the return Cdbl is: " & nr1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCdbl = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb

