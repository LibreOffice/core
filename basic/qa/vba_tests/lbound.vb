Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testLBound()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testLBound() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    Dim MyArray(1 To 10, 5 To 15, 10 To 20)     ' Declare array variables.
    testName = "Test LBound function"
    On Error GoTo errorHandler

    date2 = 1
    date1 = LBound(MyArray, 1)
    TestLog_ASSERT date1 = date2, "the return LBound is: " & date1

    date2 = 10
    date1 = LBound(MyArray, 3)
    TestLog_ASSERT date1 = date2, "the return LBound is: " & date1


    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testLBound = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


