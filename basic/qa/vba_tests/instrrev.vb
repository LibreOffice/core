Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testInStrRev()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testInStrRev() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2, SearchString, SearchChar
    testName = "Test InStrRev function"
    On Error GoTo errorHandler

    date2 = 5
    date1 = InStrRev("somemoretext", "more", -1)
    TestLog_ASSERT date1 = date2, "the return InStrRev is: " & date1

    date2 = 5
    date1 = InStrRev("somemoretext", "more")
    TestLog_ASSERT date1 = date2, "the return InStrRev is: " & date1

    date2 = 1
    date1 = InStrRev("somemoretext", "somemoretext")
    TestLog_ASSERT date1 = date2, "the return InStrRev is: " & date1

    date2 = 0
    date1 = InStrRev("somemoretext", "nothing")
    TestLog_ASSERT date1 = date2, "the return InStrRev is: " & date1

    SearchString = "XXpXXpXXPXXP"   ' String to search in.
    SearchChar = "P"    ' Search for "P".
    date2 = 3
    date1 = InStrRev(SearchString, SearchChar, 4, 1)
    TestLog_ASSERT date1 = date2, "the return InStrRev is: " & date1

    date2 = 12
    date1 = InStrRev(SearchString, SearchChar, -1, 0)
    TestLog_ASSERT date1 = date2, "the return InStrRev is: " & date1
    
    date2 = 0
    date1 = InStrRev(SearchString, "W", 1)
    TestLog_ASSERT date1 = date2, "the return InStrRev is: " & date1
    
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testInStrRev = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


