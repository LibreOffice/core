Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testOptionalsBasic
    doUnitTest = TestUtil.GetResult()
End Function

' tdf#36737 - Test optionals with different datatypes. In LO Basic, optional
' parameters are allowed, but without any default values. Missing optional
' parameters will not be initialized to their respective default values of
' its datatype, either.
Sub verify_testOptionalsBasic()
    On Error GoTo errorHandler

    ' optionals with variant datatypes
    TestUtil.AssertEqual(TestOptVariant(), 0, "TestOptVariant()")
    TestUtil.AssertEqual(TestOptVariant(123), 123, "TestOptVariant(123)")
    TestUtil.AssertEqual(TestOptVariant(, 456), 456, "TestOptVariant(, 456)")
    TestUtil.AssertEqual(TestOptVariant(123, 456), 579, "TestOptVariant(123, 456)")

    ' optionals with variant datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptVariantByRefByVal(), 0, "TestOptVariantByRefByVal()")
    TestUtil.AssertEqual(TestOptVariantByRefByVal(123), 123, "TestOptVariantByRefByVal(123)")
    TestUtil.AssertEqual(TestOptVariantByRefByVal(, 456), 456, "TestOptVariantByRefByVal(, 456)")
    TestUtil.AssertEqual(TestOptVariantByRefByVal(123, 456), 579, "TestOptVariantByRefByVal(123, 456)")

    ' optionals with double datatypes
    TestUtil.AssertEqual(TestOptDouble(), 0, "TestOptDouble()")
    TestUtil.AssertEqual(TestOptDouble(123.4), 123.4, "TestOptDouble(123.4)")
    TestUtil.AssertEqual(TestOptDouble(, 567.8), 567.8, "TestOptDouble(, 567.8)")
    TestUtil.AssertEqual(CDbl(Format(TestOptDouble(123.4, 567.8), "0.0")), 691.2, "TestOptDouble(123.4, 567.8)")

    ' optionals with double datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptDoubleByRefByVal(), 0, "TestOptDouble()")
    TestUtil.AssertEqual(TestOptDoubleByRefByVal(123.4), 123.4, "TestOptDouble(123.4)")
    TestUtil.AssertEqual(TestOptDoubleByRefByVal(, 567.8), 567.8, "TestOptDoubleByRefByVal(, 567.8)")
    TestUtil.AssertEqual(CDbl(Format(TestOptDoubleByRefByVal(123.4, 567.8), "0.0")), 691.2, "TestOptDoubleByRefByVal(123.4, 567.8)")

    ' optionals with integer datatypes
    TestUtil.AssertEqual(TestOptInteger(), 0, "TestOptInteger()")
    TestUtil.AssertEqual(TestOptInteger(123), 123, "TestOptInteger(123)")
    TestUtil.AssertEqual(TestOptInteger(, 456), 456, "TestOptInteger(, 456)")
    TestUtil.AssertEqual(TestOptInteger(123, 456), 579, "TestOptInteger(123, 456)")

    ' optionals with integer datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(), 0, "TestOptIntegerByRefByVal()")
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(123), 123, "TestOptIntegerByRefByVal(123)")
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(, 456), 456, "TestOptIntegerByRefByVal(, 456)")
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(123, 456), 579, "TestOptIntegerByRefByVal(123, 456)")

    ' optionals with string datatypes
    TestUtil.AssertEqual(TestOptString(), "", "TestOptString()")
    TestUtil.AssertEqual(TestOptString("123"), "123", "TestOptString(""123"")")
    TestUtil.AssertEqual(TestOptString(, "456"), "456", "TestOptString(, ""456"")")
    TestUtil.AssertEqual(TestOptString("123", "456"), "123456", "TestOptString(""123"", ""456"")")

    ' optionals with string datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptStringByRefByVal(), "", "TestOptStringByRefByVal()")
    TestUtil.AssertEqual(TestOptStringByRefByVal("123"), "123", "TestOptStringByRefByVal(""123"")")
    TestUtil.AssertEqual(TestOptStringByRefByVal(, "456"), "456", "TestOptStringByRefByVal(, ""456"")")
    TestUtil.AssertEqual(TestOptStringByRefByVal("123", "456"), "123456", "TestOptStringByRefByVal(""123"", ""456"")")

    ' optionals with object datatypes
    Dim cA As New Collection
    cA.Add (123)
    cA.Add (456)
    Dim cB As New Collection
    cB.Add (123.4)
    cB.Add (567.8)
    TestUtil.AssertEqual(TestOptObject(), 0, "TestOptObject()")
    TestUtil.AssertEqual(TestOptObject(cA), 579, "TestOptObject(A)")
    TestUtil.AssertEqual(CDbl(Format(TestOptObject(, cB), "0.0")), 691.2, "TestOptObject(, B)")
    TestUtil.AssertEqual(CDbl(Format(TestOptObject(cA, cB), "0.0")), 1270.2, "TestOptObject(A, B)")

    ' optionals with object datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptObjectByRefByVal(), 0, "TestOptObjectByRefByVal()")
    TestUtil.AssertEqual(TestOptObjectByRefByVal(cA), 579, "TestOptObjectByRefByVal(A)")
    TestUtil.AssertEqual(CDbl(Format(TestOptObjectByRefByVal(, cB), "0.0")), 691.2, "TestOptObjectByRefByVal(, B)")
    TestUtil.AssertEqual(CDbl(Format(TestOptObjectByRefByVal(cA, cB), "0.0")), 1270.2, "TestOptObjectByRefByVal(A, B)")

    ' optionals with array datatypes
    Dim aA(0 To 1) As Integer
    aA(0) = 123
    aA(1) = 456
    Dim aB(0 To 1) As Variant
    aB(0) = 123.4
    aB(1) = 567.8
    TestUtil.AssertEqual(TestOptArray(), 0, "TestOptArray()")
    TestUtil.AssertEqual(TestOptArray(aA), 579, "TestOptArray(A)")
    TestUtil.AssertEqual(CDbl(Format(TestOptArray(, aB), "0.0")), 691.2, "TestOptArray(, B)")
    TestUtil.AssertEqual(CDbl(Format(TestOptArray(aA, aB), "0.0")), 1270.2, "TestOptArray(A, B)")

    ' optionals with array datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptArrayByRefByVal(), 0, "TestOptArrayByRefByVal()")
    TestUtil.AssertEqual(TestOptArrayByRefByVal(aA), 579, "TestOptArrayByRefByVal(A)")
    TestUtil.AssertEqual(CDbl(Format(TestOptArrayByRefByVal(, aB), "0.0")), 691.2, "TestOptArrayByRefByVal(, B)")
    TestUtil.AssertEqual(CDbl(Format(TestOptArrayByRefByVal(aA, aB), "0.0")), 1270.2, "TestOptArrayByRefByVal(A, B)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testOptionalsBasic", Err, Error$, Erl)
End Sub

Function TestOptVariant(Optional A, Optional B As Variant)
    TestOptVariant = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptVariantByRefByVal(Optional ByRef A, Optional ByVal B As Variant)
    TestOptVariantByRefByVal = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptDouble(Optional A As Double, Optional B As Double)
    TestOptDouble = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptDoubleByRefByVal(Optional ByRef A As Double, Optional ByVal B As Double)
    TestOptDoubleByRefByVal = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptInteger(Optional A As Integer, Optional B As Integer)
    TestOptInteger = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptIntegerByRefByVal(Optional ByRef A As Integer, Optional ByVal B As Integer)
    TestOptIntegerByRefByVal = OptNumberSum(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptString(Optional A As String, Optional B As String)
    TestOptString = OptStringConcat(IsMissing(A), A, IsMissing(B), B)
End Function

Function TestOptStringByRefByVal(Optional ByRef A As String, Optional ByVal B As String)
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
