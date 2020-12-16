Rem Attribute VBA_ModuleType=VBAModule
Option VBASupport 1
Option Explicit

'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testASC()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function


Function verify_testASC() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim nr1, nr2 As Double

    testName = "Test ASC function"
    On Error GoTo errorHandler

    nr2 = 65
    nr1 = Asc("A")
    TestLog_ASSERT nr1 = nr2, "the return ASC is: " & nr1

    nr2 = 97
    nr1 = Asc("a")
    TestLog_ASSERT nr1 = nr2, "the return ASC is: " & nr1


    nr2 = 65
    nr1 = Asc("Apple")
    TestLog_ASSERT nr1 = nr2, "the return ASC is: " & nr1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testASC = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb
