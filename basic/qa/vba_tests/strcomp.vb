Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testSTRcomp()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testSTRcomp() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestStr, TestStr1, TestStr2 As String
    Dim date1, date2
    testName = "Test STRcomp function"
    On Error GoTo errorHandler
    TestStr1 = "ABCD"
    TestStr2 = "abcd"

    date2 = 0
    date1 = StrComp(TestStr1, TestStr2, vbTextCompare)
    TestLog_ASSERT date1 = date2, "the return STRcomp is: " & date1

    date2 = -1
    date1 = StrComp(TestStr1, TestStr2, vbBinaryCompare)
    TestLog_ASSERT date1 = date2, "the return STRcomp is: " & date1

    date2 = -1
    date1 = StrComp(TestStr1, TestStr2)
    TestLog_ASSERT date1 = date2, "the return STRcomp is: " & date1

    date2 = 0
    date1 = StrComp("text", "text", vbBinaryCompare)
    TestLog_ASSERT date1 = date2, "the return STRcomp is: " & date1

    date2 = 1
    date1 = StrComp("text  ", "text", vbBinaryCompare)
    TestLog_ASSERT date1 = date2, "the return STRcomp is: " & date1

    date2 = -1
    date1 = StrComp("Text", "text", vbBinaryCompare)
    TestLog_ASSERT date1 = date2, "the return STRcomp is: " & date1

    date2 = 0
    date1 = StrComp("text", "text", vbTextCompare)
    TestLog_ASSERT date1 = date2, "the return STRcomp is: " & date1

    date2 = 1
    date1 = StrComp("text  ", "text", vbTextCompare)
    TestLog_ASSERT date1 = date2, "the return STRcomp is: " & date1

    date2 = 0
    date1 = StrComp("Text", "text", vbTextCompare)
    TestLog_ASSERT date1 = date2, "the return STRcomp is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testSTRcomp = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb

