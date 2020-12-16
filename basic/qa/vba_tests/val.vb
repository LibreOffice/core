Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testVal()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testVal() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test Val function"
    On Error GoTo errorHandler

    date2 = 2
    date1 = Val("02/04/2010")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 1050
    date1 = Val("1050")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 130.75
    date1 = Val("130.75")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 50
    date1 = Val("50 Park Lane")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 1320
    date1 = Val("1320 then some text")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 0
    date1 = Val("L13.5")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

    date2 = 0
    date1 = Val("sometext")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1

REM    date2 = 1
REM    date1 = Val("1, 2")
REM    TestLog_ASSERT date1 = date2, "the return Val is: " & date1
REM		tdf#111999

    date2 = -1
    date1 = Val("&HFFFF")
    TestLog_ASSERT date1 = date2, "the return Val is: " & date1
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testVal = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


