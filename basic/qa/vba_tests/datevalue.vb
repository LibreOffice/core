Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testDateValue()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testDateValue() as String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2 As Date
    testName = "Test DateValue function"
    date2 = 25246

    On Error GoTo errorHandler

    date1 = DateValue("February 12, 1969") '2/12/1969
    TestLog_ASSERT date1 = date2, "the return date is: " & date1

    date2 = 39468
    date1 = DateValue("21/01/2008") '1/21/2008
    TestLog_ASSERT date1 = date2, "the return date is: " & date1
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testDateValue = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False),  testName & ": hit error handler"
End Sub

