Option VBASupport 1

'%%include%% _test_header.vb

Function doUnitTest() As String
    result = verify_testOptionalsVba()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = result
    Else
        doUnitTest = "OK"
    End If
End Function

' tdf#36737 - Test optionals with different datatypes. In LO Basic
' with option VBASupport, optional parameters are allowed including additional
' default values. Missing optional parameters having types other than variant,
' which don't have explicit default values, will be initialized to their
' respective default value of its datatype
Function verify_testOptionalsVba() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)
    testName = "Test optionals (VBA)"
    On Error GoTo errorHandler

    ' optionals with variant datatypes
    TestLog_ASSERT_EQUAL TestOptVariant(), 123, "TestOptVariant()"
    TestLog_ASSERT_EQUAL TestOptVariant(123), 246, "TestOptVariant(123)"
    TestLog_ASSERT_EQUAL TestOptVariant(, 456), 456, "TestOptVariant(, 456)"
    TestLog_ASSERT_EQUAL TestOptVariant(123, 456), 579, "TestOptVariant(123, 456)"

    ' optionals with variant datatypes (ByRef and ByVal)
    TestLog_ASSERT_EQUAL TestOptVariantByRefByVal(), 123, "TestOptVariantByRefByVal()"
    TestLog_ASSERT_EQUAL TestOptVariantByRefByVal(123), 246, "TestOptVariantByRefByVal(123)"
    TestLog_ASSERT_EQUAL TestOptVariantByRefByVal(, 456), 456, "TestOptVariantByRefByVal(, 456)"
    TestLog_ASSERT_EQUAL TestOptVariantByRefByVal(123, 456), 579, "TestOptVariantByRefByVal(123, 456)"

    ' optionals with double datatypes
    TestLog_ASSERT_EQUAL TestOptDouble(), 123.4, "TestOptDouble()"
    TestLog_ASSERT_EQUAL TestOptDouble(123.4), 246.8, "TestOptDouble(123.4)"
    TestLog_ASSERT_EQUAL TestOptDouble(, 567.8), 567.8, "TestOptDouble(, 567.8)"
    TestLog_ASSERT_EQUAL Format(TestOptDouble(123.4, 567.8), "0.0"), 691.2, "TestOptDouble(123.4, 567.8)"

    ' optionals with double datatypes (ByRef and ByVal)
    TestLog_ASSERT_EQUAL TestOptDoubleByRefByVal(), 123.4, "TestOptDouble()"
    TestLog_ASSERT_EQUAL TestOptDoubleByRefByVal(123.4), 246.8, "TestOptDouble(123.4)"
    TestLog_ASSERT_EQUAL TestOptDoubleByRefByVal(, 567.8), 567.8, "TestOptDoubleByRefByVal(, 567.8)"
    TestLog_ASSERT_EQUAL Format(TestOptDoubleByRefByVal(123.4, 567.8), "0.0"), 691.2, "TestOptDoubleByRefByVal(123.4, 567.8)"

    ' optionals with integer datatypes
    TestLog_ASSERT_EQUAL TestOptInteger(), 123, "TestOptInteger()"
    TestLog_ASSERT_EQUAL TestOptInteger(123), 246, "TestOptInteger(123)"
    TestLog_ASSERT_EQUAL TestOptInteger(, 456), 456, "TestOptInteger(, 456)"
    TestLog_ASSERT_EQUAL TestOptInteger(123, 456), 579, "TestOptInteger(123, 456)"

    ' optionals with integer datatypes (ByRef and ByVal)
    TestLog_ASSERT_EQUAL TestOptIntegerByRefByVal(), 123, "TestOptIntegerByRefByVal()"
    TestLog_ASSERT_EQUAL TestOptIntegerByRefByVal(123), 246, "TestOptIntegerByRefByVal(123)"
    TestLog_ASSERT_EQUAL TestOptIntegerByRefByVal(, 456), 456, "TestOptIntegerByRefByVal(, 456)"
    TestLog_ASSERT_EQUAL TestOptIntegerByRefByVal(123, 456), 579, "TestOptIntegerByRefByVal(123, 456)"

    ' optionals with string datatypes
    TestLog_ASSERT_EQUAL TestOptString(), "123", "TestOptString()"
    TestLog_ASSERT_EQUAL TestOptString("123"), "123123", "TestOptString(""123"")"
    TestLog_ASSERT_EQUAL TestOptString(, "456"), "456", "TestOptString(, ""456"")"
    TestLog_ASSERT_EQUAL TestOptString("123", "456"), "123456", "TestOptString(""123"", ""456"")"

    ' optionals with string datatypes (ByRef and ByVal)
    TestLog_ASSERT_EQUAL TestOptStringByRefByVal(), "123", "TestOptStringByRefByVal()"
    TestLog_ASSERT_EQUAL TestOptStringByRefByVal("123"), "123123", "TestOptStringByRefByVal(""123"")"
    TestLog_ASSERT_EQUAL TestOptStringByRefByVal(, "456"), "456", "TestOptStringByRefByVal(, ""456"")"
    TestLog_ASSERT_EQUAL TestOptStringByRefByVal("123", "456"), "123456", "TestOptStringByRefByVal(""123"", ""456"")"

    ' optionals with object datatypes
    Dim cA As New Collection
    cA.Add (123)
    cA.Add (456)
    Dim cB As New Collection
    cB.Add (123.4)
    cB.Add (567.8)
    TestLog_ASSERT_EQUAL TestOptObject(), 0, "TestOptObject()"
    TestLog_ASSERT_EQUAL TestOptObject(cA), 579, "TestOptObject(A)"
    TestLog_ASSERT_EQUAL Format(TestOptObject(, cB), "0.0"), 691.2, "TestOptObject(, B)"
    TestLog_ASSERT_EQUAL Format(TestOptObject(cA, cB), "0.0"), 1270.2, "TestOptObject(A, B)"

    ' optionals with object datatypes (ByRef and ByVal)
    TestLog_ASSERT_EQUAL TestOptObjectByRefByVal(), 0, "TestOptObjectByRefByVal()"
    TestLog_ASSERT_EQUAL TestOptObjectByRefByVal(cA), 579, "TestOptObjectByRefByVal(A)"
    TestLog_ASSERT_EQUAL Format(TestOptObjectByRefByVal(, cB), "0.0"), 691.2, "TestOptObjectByRefByVal(, B)"
    TestLog_ASSERT_EQUAL Format(TestOptObjectByRefByVal(cA, cB), "0.0"), 1270.2, "TestOptObjectByRefByVal(A, B)"

    ' optionals with array datatypes
    Dim aA(0 To 1) As Integer
    aA(0) = 123
    aA(1) = 456
    Dim aB(0 To 1) As Variant
    aB(0) = 123.4
    aB(1) = 567.8
    ' TODO - New bug report? Scanner initializes variable not as an array
    ' TestLog_ASSERT_EQUAL TestOptArray(), 0, "TestOptArray()"
    ' TestLog_ASSERT_EQUAL TestOptArray(aA), 579, "TestOptArray(A)"
    ' TestLog_ASSERT_EQUAL Format(TestOptArray(, aB), "0.0"), 691.2, "TestOptArray(, B)"
    TestLog_ASSERT_EQUAL Format(TestOptArray(aA, aB), "0.0"), 1270.2, "TestOptArray(A, B)"

    ' optionals with array datatypes (ByRef and ByVal)
    ' TODO - New bug report? Scanner initializes variable not as an array
    ' TestLog_ASSERT_EQUAL TestOptArrayByRefByVal(), 0, "TestOptArrayByRefByVal()"
    ' TestLog_ASSERT_EQUAL TestOptArrayByRefByVal(aA), 579, "TestOptArrayByRefByVal(A)"
    ' TestLog_ASSERT_EQUAL Format(TestOptArrayByRefByVal(, aB), "0.0"), 691.2, "TestOptArrayByRefByVal(, B)"
    TestLog_ASSERT_EQUAL Format(TestOptArrayByRefByVal(aA, aB), "0.0"), 1270.2, "TestOptArrayByRefByVal(A, B)"

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testOptionalsVba = result

    Exit Function
errorHandler:
    TestLog_ASSERT_EQUAL False, True, Err.Description
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
    ' TODO - isMissing returns false even though the collection is null and is missing?
    TestOptObject = 0
    If Not IsNull(A) Then TestOptObject = CollectionSum(A)
    If Not IsNull(B) Then TestOptObject = TestOptObject + CollectionSum(B)
End Function

Function TestOptObjectByRefByVal(Optional ByRef A As Collection, Optional ByVal B As Collection)
    ' TODO - isMissing returns false even though the collection is null and is missing?
    TestOptObjectByRefByVal = 0
    If Not IsNull(A) Then TestOptObjectByRefByVal = CollectionSum(A)
    If Not IsNull(B) Then TestOptObjectByRefByVal = TestOptObjectByRefByVal + CollectionSum(B)
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

'%%include%% _test_asserts.vb
