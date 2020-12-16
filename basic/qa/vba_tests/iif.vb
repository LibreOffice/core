Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testIIf()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testIIf() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2, testnr
    testName = "Test IIf function"
    On Error GoTo errorHandler

    date2 = "it is true"
    date1 = IIf(True, "it is true", "it is false")
    TestLog_ASSERT date1 = date2, "the return IIf is: " & date1

    date2 = "it is false"
    date1 = IIf(False, "It is true", "it is false")
    TestLog_ASSERT date1 = date2, "the return IIf is: " & date1

    testnr = 1001
    date2 = "Large"
    date1 = IIf(testnr > 1000, "Large", "Small")
    TestLog_ASSERT date1 = date2, "the return IIf is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testIIf = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb

