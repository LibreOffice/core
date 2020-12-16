Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testTypeName()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testTypeName() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test TypeName function"
    On Error GoTo errorHandler
    Dim b1 As Boolean
    Dim c1 As Byte
    Dim d1 As Date
    Dim d2 As Double
    Dim i1 As Integer
    Dim l1 As Long

    date2 = "String"
    date1 = TypeName(testName)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Boolean"
    date1 = TypeName(b1)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Byte"
    date1 = TypeName(c1)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Date"
    date1 = TypeName(d1)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Double"
    date1 = TypeName(d2)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Integer"
    date1 = TypeName(i1)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Long"
    date1 = TypeName(l1)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    ' tdf#129596 - Types of constant values
    TestLog_ASSERT TypeName(32767) = "Integer", "the return TypeName(32767) is: " & TypeName(32767)
    TestLog_ASSERT TypeName(-32767) = "Integer", "the return TypeName(-32767) is: " & TypeName(-32767)
    TestLog_ASSERT TypeName(1048575) = "Long", "the return TypeName(1048575) is: " & TypeName(1048575)
    TestLog_ASSERT TypeName(-1048575) = "Long", "the return TypeName(-1048575) is: " & TypeName(-1048575)

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testTypeName = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

'%%include%% _test_asserts.vb

