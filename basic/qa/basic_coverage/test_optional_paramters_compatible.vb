Option Compatible

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
    result = verify_testOptionalsCompatible()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function

' tdf#36737 - Test optionals with different datatypes. In LO Basic
' with option Compatible, optional parameters are allowed with default values.
' Missing optional parameters that don't have explicit default values will
' not be initialized to their default values of its datatype.
Function verify_testOptionalsCompatible() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)
    testName = "Test optionals (Compatible)"
    On Error GoTo errorHandler

    ' optionals with variant datatypes
    TestLog_ASSERT TestOptVariant(), 123, "TestOptVariant()"
    TestLog_ASSERT TestOptVariant(123), 246, "TestOptVariant(123)"
    TestLog_ASSERT TestOptVariant(, 456), 456, "TestOptVariant(, 456)"
    TestLog_ASSERT TestOptVariant(123, 456), 579, "TestOptVariant(123, 456)"

    ' optionals with variant datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptVariantByRefByVal(), 123, "TestOptVariantByRefByVal()"
    TestLog_ASSERT TestOptVariantByRefByVal(123), 246, "TestOptVariantByRefByVal(123)"
    TestLog_ASSERT TestOptVariantByRefByVal(, 456), 456, "TestOptVariantByRefByVal(, 456)"
    TestLog_ASSERT TestOptVariantByRefByVal(123, 456), 579, "TestOptVariantByRefByVal(123, 456)"

    ' optionals with double datatypes
    TestLog_ASSERT TestOptDouble(), 123.4, "TestOptDouble()"
    TestLog_ASSERT TestOptDouble(123.4), 246.8, "TestOptDouble(123.4)"
    TestLog_ASSERT TestOptDouble(, 567.8), 567.8, "TestOptDouble(, 567.8)"
    TestLog_ASSERT CDbl(Format(TestOptDouble(123.4, 567.8), "0.0")), 691.2, "TestOptDouble(123.4, 567.8)"

    ' optionals with double datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptDoubleByRefByVal(), 123.4, "TestOptDouble()"
    TestLog_ASSERT TestOptDoubleByRefByVal(123.4), 246.8, "TestOptDouble(123.4)"
    TestLog_ASSERT TestOptDoubleByRefByVal(, 567.8), 567.8, "TestOptDoubleByRefByVal(, 567.8)"
    TestLog_ASSERT CDbl(Format(TestOptDoubleByRefByVal(123.4, 567.8), "0.0")), 691.2, "TestOptDoubleByRefByVal(123.4, 567.8)"

    ' optionals with integer datatypes
    TestLog_ASSERT TestOptInteger(), 123, "TestOptInteger()"
    TestLog_ASSERT TestOptInteger(123), 246, "TestOptInteger(123)"
    TestLog_ASSERT TestOptInteger(, 456), 456, "TestOptInteger(, 456)"
    TestLog_ASSERT TestOptInteger(123, 456), 579, "TestOptInteger(123, 456)"

    ' optionals with integer datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptIntegerByRefByVal(), 123, "TestOptIntegerByRefByVal()"
    TestLog_ASSERT TestOptIntegerByRefByVal(123), 246, "TestOptIntegerByRefByVal(123)"
    TestLog_ASSERT TestOptIntegerByRefByVal(, 456), 456, "TestOptIntegerByRefByVal(, 456)"
    TestLog_ASSERT TestOptIntegerByRefByVal(123, 456), 579, "TestOptIntegerByRefByVal(123, 456)"

    ' optionals with string datatypes
    TestLog_ASSERT TestOptString(), "123", "TestOptString()"
    TestLog_ASSERT TestOptString("123"), "123123", "TestOptString(""123"")"
    TestLog_ASSERT TestOptString(, "456"), "456", "TestOptString(, ""456"")"
    TestLog_ASSERT TestOptString("123", "456"), "123456", "TestOptString(""123"", ""456"")"

    ' optionals with string datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptStringByRefByVal(), "123", "TestOptStringByRefByVal()"
    TestLog_ASSERT TestOptStringByRefByVal("123"), "123123", "TestOptStringByRefByVal(""123"")"
    TestLog_ASSERT TestOptStringByRefByVal(, "456"), "456", "TestOptStringByRefByVal(, ""456"")"
    TestLog_ASSERT TestOptStringByRefByVal("123", "456"), "123456", "TestOptStringByRefByVal(""123"", ""456"")"

    ' optionals with object datatypes
    Dim cA As New Collection
    cA.Add (123)
    cA.Add (456)
    Dim cB As New Collection
    cB.Add (123.4)
    cB.Add (567.8)
    TestLog_ASSERT TestOptObject(), 0, "TestOptObject()"
    TestLog_ASSERT TestOptObject(cA), 579, "TestOptObject(A)"
    TestLog_ASSERT CDbl(Format(TestOptObject(, cB), "0.0")), 691.2, "TestOptObject(, B)"
    TestLog_ASSERT CDbl(Format(TestOptObject(cA, cB), "0.0")), 1270.2, "TestOptObject(A, B)"

    ' optionals with object datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptObjectByRefByVal(), 0, "TestOptObjectByRefByVal()"
    TestLog_ASSERT TestOptObjectByRefByVal(cA), 579, "TestOptObjectByRefByVal(A)"
    TestLog_ASSERT CDbl(Format(TestOptObjectByRefByVal(, cB), "0.0")), 691.2, "TestOptObjectByRefByVal(, B)"
    TestLog_ASSERT CDbl(Format(TestOptObjectByRefByVal(cA, cB), "0.0")), 1270.2, "TestOptObjectByRefByVal(A, B)"

    ' optionals with array datatypes
    Dim aA(0 To 1) As Integer
    aA(0) = 123
    aA(1) = 456
    Dim aB(0 To 1) As Variant
    aB(0) = 123.4
    aB(1) = 567.8
    TestLog_ASSERT TestOptArray(), 0, "TestOptArray()"
    TestLog_ASSERT TestOptArray(aA), 579, "TestOptArray(A)"
    TestLog_ASSERT CDbl(Format(TestOptArray(, aB), "0.0")), 691.2, "TestOptArray(, B)"
    TestLog_ASSERT CDbl(Format(TestOptArray(aA, aB), "0.0")), 1270.2, "TestOptArray(A, B)"

    ' optionals with array datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptArrayByRefByVal(), 0, "TestOptArrayByRefByVal()"
    TestLog_ASSERT TestOptArrayByRefByVal(aA), 579, "TestOptArrayByRefByVal(A)"
    TestLog_ASSERT CDbl(Format(TestOptArrayByRefByVal(, aB), "0.0")), 691.2, "TestOptArrayByRefByVal(, B)"
    TestLog_ASSERT CDbl(Format(TestOptArrayByRefByVal(aA, aB), "0.0")), 1270.2, "TestOptArrayByRefByVal(A, B)"

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testOptionalsCompatible = result

    Exit Function
errorHandler:
    TestLog_ASSERT False, True, Err.Description
End Function

Function TestOptVariant(Optional A, Optional B As Variant = 123)
    TestOptVariant = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptVariantByRefByVal(Optional ByRef A, Optional ByVal B As Variant = 123)
    TestOptVariantByRefByVal = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptDouble(Optional A As Double, Optional B As Double = 123.4)
    TestOptDouble = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptDoubleByRefByVal(Optional ByRef A As Double, Optional ByVal B As Double = 123.4)
    TestOptDoubleByRefByVal = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptInteger(Optional A As Integer, Optional B As Integer = 123)
    TestOptInteger = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptIntegerByRefByVal(Optional ByRef A As Integer, Optional ByVal B As Integer = 123)
    TestOptIntegerByRefByVal = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptString(Optional A As String, Optional B As String = "123")
    TestOptString = OptStringConcat(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptStringByRefByVal(Optional ByRef A As String, Optional ByVal B As String = "123")
    TestOptStringByRefByVal = OptStringConcat(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptObject(Optional A As Collection, Optional B As Collection)
    TestOptObject = 0
    If Not IsMissing(A) Then TestOptObject = CollectionSum(A)
    If Not IsMissing(B) Then TestOptObject = TestOptObject + CollectionSum(B)
End Function

Function TestOptObjectByRefByVal(Optional ByRef A As Collection, Optional ByVal B As Collection)
    TestOptObjectByRefByVal = 0
    If Not IsMissing(A) Then TestOptObjectByRefByVal = CollectionSum(A)
    If Not IsMissing(B) Then TestOptObjectByRefByVal = TestOptObjectByRefByVal + CollectionSum(B)
End Function

Function TestOptArray(Optional A() As Integer, Optional B() As Variant)
    TestOptArray = ArraySum(IsMissing(A), A) + ArraySum(IsMissing(B), B)
End Function

Function TestOptArrayByRefByVal(Optional ByRef A() As Integer, Optional ByVal B() As Variant)
    TestOptArrayByRefByVal = ArraySum(IsMissing(A), A) + ArraySum(IsMissing(B), B)
End Function

Function OptNumberSum(is_missingA As Boolean, A, is_missingB As Boolean, B)
    OptNumberSum = 0
    If Not is_missingA Then OptNumberSum = A
    If Not is_missingB Then OptNumberSum = OptNumberSum + B
End Function

Function OptStringConcat(is_missingA As Boolean, A, is_missingB As Boolean, B)
    OptStringConcat = ""
    If Not is_missingA Then OptStringConcat = A
    If Not is_missingB Then OptStringConcat = OptStringConcat & B
End Function

Function CollectionSum(C)
    Dim idx As Integer
    CollectionSum = 0
    For idx = 1 To C.Count
        CollectionSum = CollectionSum + C.Item(idx)
    Next idx
End Function

Function ArraySum(is_missingC As Boolean, C)
    Dim idx As Integer
    ArraySum = 0
    If Not is_missingC Then
        For idx = LBound(C) To UBound(C)
            ArraySum = ArraySum + C(idx)
        Next idx
    End If
End Function

Sub TestLog_ASSERT(actual As Variant, expected As Variant, testName As String)
    If expected = actual Then
        passCount = passCount + 1
    Else
        result = result & Chr$(10) & " Failed: " & testName & " returned " & actual & ", expected " & expected
        failCount = failCount + 1
    End If
End Sub