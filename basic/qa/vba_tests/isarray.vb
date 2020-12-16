Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testIsArray()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testIsArray() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    Dim MyArray(1 To 5) As Integer, YourArray    ' Declare array variables.
    testName = "Test IsArray function"
    On Error GoTo errorHandler
    YourArray = Array(1, 2, 3)    ' Use Array function.

    date2 = True
    date1 = IsArray(MyArray)
    TestLog_ASSERT date1 = date2, "the return IsArray is: " & date1

    date2 = True
    date1 = IsArray(YourArray)
    TestLog_ASSERT date1 = date2, "the return IsArray is: " & date1

    date2 = False
    date1 = IsArray(date2)
    TestLog_ASSERT date1 = date2, "the return IsArray is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testIsArray = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb

