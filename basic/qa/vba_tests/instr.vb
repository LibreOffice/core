Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testInStr()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testInStr() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2, SearchString, SearchChar
    testName = "Test InStr function"
    On Error GoTo errorHandler

    date2 = 5
    date1 = InStr(1, "somemoretext", "more")
    TestLog_ASSERT date1 = date2, "the return InStr is: " & date1

    date2 = 5
    date1 = InStr("somemoretext", "more")
    TestLog_ASSERT date1 = date2, "the return InStr is: " & date1

    date2 = 1
    date1 = InStr("somemoretext", "somemoretext")
    TestLog_ASSERT date1 = date2, "the return InStr is: " & date1

    date2 = 0
    date1 = InStr("somemoretext", "nothing")
    TestLog_ASSERT date1 = date2, "the return InStr is: " & date1

    SearchString = "XXpXXpXXPXXP"   ' String to search in.
    SearchChar = "P"    ' Search for "P".
    date2 = 6
    date1 = InStr(4, SearchString, SearchChar, 1)
    TestLog_ASSERT date1 = date2, "the return InStr is: " & date1

    date2 = 9
    date1 = InStr(1, SearchString, SearchChar, 0)
    TestLog_ASSERT date1 = date2, "the return InStr is: " & date1
    
    date2 = 0
    date1 = InStr(1, SearchString, "W")
    TestLog_ASSERT date1 = date2, "the return InStr is: " & date1
    
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testInStr = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


