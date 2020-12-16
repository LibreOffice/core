Option VBASupport 1
Rem Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testRGB()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testRGB() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim date1, date2
    testName = "Test RGB function"
    On Error GoTo errorHandler

    date2 = 255
    date1 = RGB(255, 0, 0)
    TestLog_ASSERT date1 = date2, "the return RGB is: " & date1

    date2 = 13339467
    date1 = RGB(75, 139, 203)
    TestLog_ASSERT date1 = date2, "the return RGB is: " & date1

    date2 = 16777215
    date1 = RGB(255, 255, 255)
    TestLog_ASSERT date1 = date2, "the return RGB is: " & date1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testRGB = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb

