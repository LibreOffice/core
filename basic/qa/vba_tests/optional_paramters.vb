Option VBASupport 1

Function doUnitTest() As String
    verify_testOptionalsVba
    doUnitTest = TestUtilModule.GetResult()
End Function

' tdf#36737 - Test optionals with different datatypes. In LO Basic
' with option VBASupport, optional parameters are allowed including additional
' default values. Missing optional parameters having types other than variant,
' which don't have explicit default values, will be initialized to their
' respective default value of its datatype
Sub verify_testOptionalsVba()

    TestUtilModule.TestInit
    testName = "Test optionals (VBA)"
    On Error GoTo errorHandler

    ' optionals with variant datatypes
    TestUtilModule.AssertEqual(TestOptVariant(), 123, "TestOptVariant()")
    TestUtilModule.AssertEqual(TestOptVariant(123), 246, "TestOptVariant(123)")
    TestUtilModule.AssertEqual(TestOptVariant(, 456), 456, "TestOptVariant(, 456)")
    TestUtilModule.AssertEqual(TestOptVariant(123, 456), 579, "TestOptVariant(123, 456)")

    ' optionals with variant datatypes (ByRef and ByVal)
    TestUtilModule.AssertEqual(TestOptVariantByRefByVal(), 123, "TestOptVariantByRefByVal()")
    TestUtilModule.AssertEqual(TestOptVariantByRefByVal(123), 246, "TestOptVariantByRefByVal(123)")
    TestUtilModule.AssertEqual(TestOptVariantByRefByVal(, 456), 456, "TestOptVariantByRefByVal(, 456)")
    TestUtilModule.AssertEqual(TestOptVariantByRefByVal(123, 456), 579, "TestOptVariantByRefByVal(123, 456)")

    ' optionals with double datatypes
    TestUtilModule.AssertEqual(TestOptDouble(), 123.4, "TestOptDouble()")
    TestUtilModule.AssertEqual(TestOptDouble(123.4), 246.8, "TestOptDouble(123.4)")
    TestUtilModule.AssertEqual(TestOptDouble(, 567.8), 567.8, "TestOptDouble(, 567.8)")
    TestUtilModule.AssertEqual(Format(TestOptDouble(123.4, 567.8), "0.0"), 691.2, "TestOptDouble(123.4, 567.8)")

    ' optionals with double datatypes (ByRef and ByVal)
    TestUtilModule.AssertEqual(TestOptDoubleByRefByVal(), 123.4, "TestOptDouble()")
    TestUtilModule.AssertEqual(TestOptDoubleByRefByVal(123.4), 246.8, "TestOptDouble(123.4)")
    TestUtilModule.AssertEqual(TestOptDoubleByRefByVal(, 567.8), 567.8, "TestOptDoubleByRefByVal(, 567.8)")
    TestUtilModule.AssertEqual(Format(TestOptDoubleByRefByVal(123.4, 567.8), "0.0"), 691.2, "TestOptDoubleByRefByVal(123.4, 567.8)")

    ' optionals with integer datatypes
    TestUtilModule.AssertEqual(TestOptInteger(), 123, "TestOptInteger()")
    TestUtilModule.AssertEqual(TestOptInteger(123), 246, "TestOptInteger(123)")
    TestUtilModule.AssertEqual(TestOptInteger(, 456), 456, "TestOptInteger(, 456)")
    TestUtilModule.AssertEqual(TestOptInteger(123, 456), 579, "TestOptInteger(123, 456)")

    ' optionals with integer datatypes (ByRef and ByVal)
    TestUtilModule.AssertEqual(TestOptIntegerByRefByVal(), 123, "TestOptIntegerByRefByVal()")
    TestUtilModule.AssertEqual(TestOptIntegerByRefByVal(123), 246, "TestOptIntegerByRefByVal(123)")
    TestUtilModule.AssertEqual(TestOptIntegerByRefByVal(, 456), 456, "TestOptIntegerByRefByVal(, 456)")
    TestUtilModule.AssertEqual(TestOptIntegerByRefByVal(123, 456), 579, "TestOptIntegerByRefByVal(123, 456)")

    ' optionals with string datatypes
    TestUtilModule.AssertEqual(TestOptString(), "123", "TestOptString()")
    TestUtilModule.AssertEqual(TestOptString("123"), "123123", "TestOptString(""123"")")
    TestUtilModule.AssertEqual(TestOptString(, "456"), "456", "TestOptString(, ""456"")")
    TestUtilModule.AssertEqual(TestOptString("123", "456"), "123456", "TestOptString(""123"", ""456"")")

    ' optionals with string datatypes (ByRef and ByVal)
    TestUtilModule.AssertEqual(TestOptStringByRefByVal(), "123", "TestOptStringByRefByVal()")
    TestUtilModule.AssertEqual(TestOptStringByRefByVal("123"), "123123", "TestOptStringByRefByVal(""123"")")
    TestUtilModule.AssertEqual(TestOptStringByRefByVal(, "456"), "456", "TestOptStringByRefByVal(, ""456"")")
    TestUtilModule.AssertEqual(TestOptStringByRefByVal("123", "456"), "123456", "TestOptStringByRefByVal(""123"", ""456"")")

    ' optionals with object datatypes
    Dim cA As New Collection
    cA.Add (123)
    cA.Add (456)
    Dim cB As New Collection
    cB.Add (123.4)
    cB.Add (567.8)
    TestUtilModule.AssertEqual(TestOptObject(), 0, "TestOptObject()")
    TestUtilModule.AssertEqual(TestOptObject(cA), 579, "TestOptObject(A)")
    TestUtilModule.AssertEqual(Format(TestOptObject(, cB), "0.0"), 691.2, "TestOptObject(, B)")
    TestUtilModule.AssertEqual(Format(TestOptObject(cA, cB), "0.0"), 1270.2, "TestOptObject(A, B)")

    ' optionals with object datatypes (ByRef and ByVal)
    TestUtilModule.AssertEqual(TestOptObjectByRefByVal(), 0, "TestOptObjectByRefByVal()")
    TestUtilModule.AssertEqual(TestOptObjectByRefByVal(cA), 579, "TestOptObjectByRefByVal(A)")
    TestUtilModule.AssertEqual(Format(TestOptObjectByRefByVal(, cB), "0.0"), 691.2, "TestOptObjectByRefByVal(, B)")
    TestUtilModule.AssertEqual(Format(TestOptObjectByRefByVal(cA, cB), "0.0"), 1270.2, "TestOptObjectByRefByVal(A, B)")

    ' optionals with array datatypes
    Dim aA(0 To 1) As Integer
    aA(0) = 123
    aA(1) = 456
    Dim aB(0 To 1) As Variant
    aB(0) = 123.4
    aB(1) = 567.8
    ' TODO - New bug report? Scanner initializes variable not as an array
    ' TestUtilModule.AssertEqual(TestOptArray(), 0, "TestOptArray()")
    ' TestUtilModule.AssertEqual(TestOptArray(aA), 579, "TestOptArray(A)")
    ' TestUtilModule.AssertEqual(Format(TestOptArray(, aB), "0.0"), 691.2, "TestOptArray(, B)")
    TestUtilModule.AssertEqual(Format(TestOptArray(aA, aB), "0.0"), 1270.2, "TestOptArray(A, B)")

    ' optionals with array datatypes (ByRef and ByVal)
    ' TODO - New bug report? Scanner initializes variable not as an array
    ' TestUtilModule.AssertEqual(TestOptArrayByRefByVal(), 0, "TestOptArrayByRefByVal()")
    ' TestUtilModule.AssertEqual(TestOptArrayByRefByVal(aA), 579, "TestOptArrayByRefByVal(A)")
    ' TestUtilModule.AssertEqual(Format(TestOptArrayByRefByVal(, aB), "0.0"), 691.2, "TestOptArrayByRefByVal(, B)")
    TestUtilModule.AssertEqual(Format(TestOptArrayByRefByVal(aA, aB), "0.0"), 1270.2, "TestOptArrayByRefByVal(A, B)")

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
    TestUtilModule.AssertEqual(False, True, Err.Description)
End Sub

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

