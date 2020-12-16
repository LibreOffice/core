Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

Function doUnitTest() As String
result = verify_testIsObject()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testIsObject() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestStr As String
    Dim MyObject As Object
    Dim date1, date2, YourObject
    testName = "Test IsObject function"
    On Error GoTo errorHandler

    Set YourObject = MyObject    ' Assign an object reference.
    date2 = True
    date1 = IsObject(YourObject)
    TestLog_ASSERT date1 = date2, "the return IsObject is: " & date1

    date2 = False
    date1 = IsObject(TestStr)
    TestLog_ASSERT date1 = date2, "the return IsObject is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testIsObject = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function


