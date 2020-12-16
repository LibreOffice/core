Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testConstants()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_testConstants() As String
    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    testName = "Test Constants"
    On Error GoTo errorHandler

    If GetGuiType() = 1 Then
        TestLog_ASSERT vbNewline = vbCrLf, "vbNewLine is the same as vbCrLf on Windows"
    Else
        TestLog_ASSERT vbNewLine = vbLf, "vbNewLine is the same as vbLf on others than Windows"
    End If

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testConstants = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb
