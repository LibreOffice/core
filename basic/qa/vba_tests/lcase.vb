Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testLCase()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testLCase() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim str1, str2 As String 'variables for test
    testName = "Test LCase function"
    On Error GoTo errorHandler

    str2 = "lowercase"
    str1 = LCase("LOWERCASE")
    TestLog_ASSERT str1 = str2, "the return LCase is: " & str1

    str2 = "lowercase"
    str1 = LCase("LowerCase")
    TestLog_ASSERT str1 = str2, "the return LCase is: " & str1

    str2 = "lowercase"
    str1 = LCase("lowercase")
    TestLog_ASSERT str1 = str2, "the return LCase is: " & str1

    str2 = "lower case"
    str1 = LCase("LOWER CASE")
    TestLog_ASSERT str1 = str2, "the return LCase is: " & str1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testLCase = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


