Option Base 1

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
    result = verify_optionBase()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function

Function verify_optionBase() As String
    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    ' tdf#54912 - with option base arrays should start at index 1
    Dim strArray(2) As String
    strArray = Array("a", "b")
    TestLog_ASSERT LBound(strArray), 1, "Lower bound of a string array: " & LBound(strArray)

    Dim intArray(2) As Integer
    intArray = Array(1, 2)
    TestLog_ASSERT LBound(intArray), 1, "Lower bound of an integer array: " & LBound(intArray)

    Dim byteArray() As Byte
    byteArray = StrConv("ab", 128)
    TestLog_ASSERT LBound(byteArray), 1, "Lower bound of a byte array (StrConv): " & LBound(byteArray)

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_optionBase = result
End Function

Sub TestLog_ASSERT(actual As Variant, expected As Variant, testName As String)
    If expected = actual Then
        passCount = passCount + 1
    Else
        result = result & Chr$(10) & "Failed: " & testName & " returned " & actual & ", expected " & expected
        failCount = failCount + 1
    End If
End Sub
