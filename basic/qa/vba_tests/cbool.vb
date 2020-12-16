Option VBASupport 1
Rem Option VBASupport 1    'unREM in .vb file
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testCBool()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testCBool() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim res2, res1 As Boolean
    Dim a1, a2 As Integer
    testName = "Test CBool function"
    On Error GoTo errorHandler

    res2 = True
    res1 = CBool(1)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = False
    res1 = CBool(1 = 2)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = False
    res1 = CBool(0)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = True
    res1 = CBool(21)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = True
    res1 = CBool("true")
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = False
    res1 = CBool("false")
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = True
    res1 = CBool("1")
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = True
    res1 = CBool("-1")
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = False
    res1 = CBool("0")
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = False
    a1 = 1: a2 = 10
    res1 = CBool(a1 = a2)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    res2 = True
    a1 = 10: a2 = 10
    res1 = CBool(a1 = a2)
    TestLog_ASSERT res1 = res2, "the return CBool is: " & res1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCBool = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

