Option Base 1
Option Compatible

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

    ' tdf#54912 - with option base arrays should start at index 1.
    ' With option compatible the upper bound is not changed (#109275).
    Dim strArray(2) As String
    TestLog_ASSERT LBound(strArray), 1, "Lower bound of a string array (before assignment): " & LBound(strArray)
    TestLog_ASSERT UBound(strArray), 2, "Upper bound of a string array (before assignment): " & UBound(strArray)
    strArray = Array("a", "b")
    TestLog_ASSERT LBound(strArray), 1, "Lower bound of a string array (after assignment): " & LBound(strArray)
    TestLog_ASSERT UBound(strArray), 2, "Upper bound of a string array (after assignment): " & UBound(strArray)

    Dim intArray(2) As Integer
    TestLog_ASSERT LBound(intArray), 1, "Lower bound of an integer array (before assignment): " & LBound(intArray)
    TestLog_ASSERT UBound(intArray), 2, "Upper bound of an integer array (before assignment): " & UBound(intArray)
    intArray = Array(1, 2)
    TestLog_ASSERT LBound(intArray), 1, "Lower bound of an integer array (after assignment): " & LBound(intArray)
    TestLog_ASSERT UBound(intArray), 2, "Upper bound of an integer array (after assignment): " & UBound(intArray)

    Dim byteArray(2) As Byte
    TestLog_ASSERT LBound(byteArray), 1, "Lower bound of a byte array (before assignment): " & LBound(byteArray)
    TestLog_ASSERT UBound(byteArray), 2, "Upper bound of a byte array (before assignment): " & UBound(byteArray)
    byteArray = StrConv("ab", 128)
    TestLog_ASSERT LBound(byteArray), 1, "Lower bound of a byte array (StrConv): " & LBound(byteArray)
    TestLog_ASSERT UBound(byteArray), 2, "Upper bound of a byte array (StrConv): " & UBound(byteArray)

    ReDim byteArray(3)
    TestLog_ASSERT LBound(byteArray), 1, "Lower bound of a byte array (ReDim): " & LBound(byteArray)
    TestLog_ASSERT UBound(byteArray), 3, "Upper bound of a byte array (ReDim): " & UBound(byteArray)

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
