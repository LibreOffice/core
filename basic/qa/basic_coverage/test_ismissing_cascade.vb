Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
    result = verify_testIsMissingCascade()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function

Function verify_testIsMissingCascade() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)
    testName = "Test missing (IsMissing with cascading optionals)"
    On Error GoTo errorHandler

    ' tdf#136143 - test cascading optionals in order to prevent type conversion errors, because
    ' optional arguments are of type SbxERROR and set to not fixed.
    TestLog_ASSERT TestOpt(), 2, "Cascading optionals"

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testIsMissingCascade = result

    Exit Function
errorHandler:
    TestLog_ASSERT False, True, Err.Description
End Function

Function TestOpt(Optional A)
    TestOpt = TestOptCascade(A)
End Function

Function TestOptCascade(Optional A)
    If IsMissing(A) Then A = 2
    TestOptCascade = A
End Function

Sub TestLog_ASSERT(actual As Variant, expected As Integer, testName As String)
    If expected = actual Then
        passCount = passCount + 1
    Else
        result = result & Chr$(10) & " Failed: " & testName & " returned " & actual & ", expected " & expected
        failCount = failCount + 1
    End If
End Sub