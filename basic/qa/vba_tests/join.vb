Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testJoin()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testJoin() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim str1, str2 As String
    Dim vaArray(2) As String
    testName = "Test Join function"
    On Error GoTo errorHandler
    vaArray(0) = "string1"
    vaArray(1) = "string2"
    vaArray(2) = "string3"

    str2 = "string1 string2 string3"
    str1 = Join(vaArray)
    TestLog_ASSERT str1 = str2, "the return Join is: " & str1

    str2 = "string1 string2 string3"
    str1 = Join(vaArray, " ")
    TestLog_ASSERT str1 = str2, "the return Join is: " & str1

    str2 = "string1<>string2<>string3"
    str1 = Join(vaArray, "<>")
    TestLog_ASSERT str1 = str2, "the return Join is: " & str1

    str2 = "string1string2string3"
    str1 = Join(vaArray, "")
    TestLog_ASSERT str1 = str2, "the return Join is: " & str1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testJoin = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb

