Option Compatible
Option Explicit

Type testObject
    testInt As Integer
End Type

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testOptionalsCompatible
    doUnitTest = TestUtil.GetResult()
End Function

' tdf#36737 - Test optionals with different datatypes. In LO Basic
' with option Compatible, optional parameters are allowed with default values.
' Missing optional parameters that don't have explicit default values will
' not be initialized to their default values of its datatype.
Sub verify_testOptionalsCompatible()

    On Error GoTo errorHandler

    ' optionals with variant datatypes
    TestUtil.AssertEqual(TestOptVariant(), 123, "TestOptVariant()")
    TestUtil.AssertEqual(TestOptVariant(123), 246, "TestOptVariant(123)")
    TestUtil.AssertEqual(TestOptVariant(, 456), 456, "TestOptVariant(, 456)")
    TestUtil.AssertEqual(TestOptVariant(123, 456), 579, "TestOptVariant(123, 456)")

    ' optionals with variant datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptVariantByRefByVal(), 123, "TestOptVariantByRefByVal()")
    TestUtil.AssertEqual(TestOptVariantByRefByVal(123), 246, "TestOptVariantByRefByVal(123)")
    TestUtil.AssertEqual(TestOptVariantByRefByVal(, 456), 456, "TestOptVariantByRefByVal(, 456)")
    TestUtil.AssertEqual(TestOptVariantByRefByVal(123, 456), 579, "TestOptVariantByRefByVal(123, 456)")

    ' optionals with double datatypes
    TestUtil.AssertEqualApprox(TestOptDouble(), 123.4, 1E-5, "TestOptDouble()")
    TestUtil.AssertEqualApprox(TestOptDouble(123.4), 246.8, 1E-5, "TestOptDouble(123.4)")
    TestUtil.AssertEqualApprox(TestOptDouble(, 567.8), 567.8, 1E-5, "TestOptDouble(, 567.8)")
    TestUtil.AssertEqualApprox(TestOptDouble(123.4, 567.8), 691.2, 1E-5, "TestOptDouble(123.4, 567.8)")

    ' optionals with double datatypes (ByRef and ByVal)
    TestUtil.AssertEqualApprox(TestOptDoubleByRefByVal(), 123.4, 1E-5, "TestOptDoubleByRefByVal()")
    TestUtil.AssertEqualApprox(TestOptDoubleByRefByVal(123.4), 246.8, 1E-5, "TestOptDoubleByRefByVal(123.4)")
    TestUtil.AssertEqualApprox(TestOptDoubleByRefByVal(, 567.8), 567.8, 1E-5, "TestOptDoubleByRefByVal(, 567.8)")
    TestUtil.AssertEqualApprox(TestOptDoubleByRefByVal(123.4, 567.8), 691.2, 1E-5, "TestOptDoubleByRefByVal(123.4, 567.8)")

    ' optionals with integer datatypes
    TestUtil.AssertEqual(TestOptInteger(), 123, "TestOptInteger()")
    TestUtil.AssertEqual(TestOptInteger(123), 246, "TestOptInteger(123)")
    TestUtil.AssertEqual(TestOptInteger(, 456), 456, "TestOptInteger(, 456)")
    TestUtil.AssertEqual(TestOptInteger(123, 456), 579, "TestOptInteger(123, 456)")

    ' optionals with integer datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(), 123, "TestOptIntegerByRefByVal()")
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(123), 246, "TestOptIntegerByRefByVal(123)")
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(, 456), 456, "TestOptIntegerByRefByVal(, 456)")
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(123, 456), 579, "TestOptIntegerByRefByVal(123, 456)")

    ' optionals with string datatypes
    TestUtil.AssertEqual(TestOptString(), "123", "TestOptString()")
    TestUtil.AssertEqual(TestOptString("123"), "123123", "TestOptString(""123"")")
    TestUtil.AssertEqual(TestOptString(, "456"), "456", "TestOptString(, ""456"")")
    TestUtil.AssertEqual(TestOptString("123", "456"), "123456", "TestOptString(""123"", ""456"")")

    ' optionals with string datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptStringByRefByVal(), "123", "TestOptStringByRefByVal()")
    TestUtil.AssertEqual(TestOptStringByRefByVal("123"), "123123", "TestOptStringByRefByVal(""123"")")
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
    TestUtil.AssertEqualApprox(TestOptObject(, cB), 691.2, 1E-5, "TestOptObject(, B)")
    TestUtil.AssertEqualApprox(TestOptObject(cA, cB), 1270.2, 1E-5, "TestOptObject(A, B)")

    ' optionals with object datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptObjectByRefByVal(), 0, "TestOptObjectByRefByVal()")
    TestUtil.AssertEqual(TestOptObjectByRefByVal(cA), 579, "TestOptObjectByRefByVal(A)")
    TestUtil.AssertEqualApprox(TestOptObjectByRefByVal(, cB), 691.2, 1E-5, "TestOptObjectByRefByVal(, B)")
    TestUtil.AssertEqualApprox(TestOptObjectByRefByVal(cA, cB), 1270.2, 1E-5, "TestOptObjectByRefByVal(A, B)")

    ' optionals with array datatypes
    Dim aA(0 To 1) As Integer
    aA(0) = 123
    aA(1) = 456
    Dim aB(0 To 1) As Variant
    aB(0) = 123.4
    aB(1) = 567.8
    TestUtil.AssertEqual(TestOptArray(), 0, "TestOptArray()")
    TestUtil.AssertEqual(TestOptArray(aA), 579, "TestOptArray(A)")
    TestUtil.AssertEqualApprox(TestOptArray(, aB), 691.2, 1E-5, "TestOptArray(, B)")
    TestUtil.AssertEqualApprox(TestOptArray(aA, aB), 1270.2, 1E-5, "TestOptArray(A, B)")

    ' optionals with array datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptArrayByRefByVal(), 0, "TestOptArrayByRefByVal()")
    TestUtil.AssertEqual(TestOptArrayByRefByVal(aA), 579, "TestOptArrayByRefByVal(A)")
    TestUtil.AssertEqualApprox(TestOptArrayByRefByVal(, aB), 691.2, 1E-5, "TestOptArrayByRefByVal(, B)")
    TestUtil.AssertEqualApprox(TestOptArrayByRefByVal(aA, aB), 1270.2, 1E-5, "TestOptArrayByRefByVal(A, B)")

    ' tdf#144353 - error handling of missing optional parameters (arithmetic operator)
    ' Without the fix in place, this test would have failed with:
    ' - Expected: 449 (ERRCODE_BASIC_NOT_OPTIONAL - Argument not optional)
    ' - Actual  : 549 (Actual value of the variable)
    TestUtil.AssertEqual(TestArithmeticOperator, 449, "TestArithmeticOperator")

    ' tdf#144353 - error handling of missing optional parameters (unary operator)
    ' Without the fix in place, this test would have failed with:
    ' - Expected: 449 (ERRCODE_BASIC_NOT_OPTIONAL - Argument not optional)
    ' - Actual  : 100 (Actual value of the variable)
    TestUtil.AssertEqual(TestUnaryOperator, 449, "TestUnaryOperator")

    ' tdf#144353 - error handling of missing optional parameters (assigning to a collection)
    ' Without the fix in place, this test would have failed with:
    ' - Expected: 449 (ERRCODE_BASIC_NOT_OPTIONAL - Argument not optional)
    ' - Actual  : 549 (Actual value of the variable)
    TestUtil.AssertEqual(TestCollection, 449, "TestCollection")

    ' tdf#144353 - error handling of missing optional parameters (assigning to an object)
    ' Without the fix in place, this test would have failed with:
    ' - Expected: 449 (ERRCODE_BASIC_NOT_OPTIONAL - Argument not optional)
    ' - Actual  : 448 (Actual value of the variable)
    TestUtil.AssertEqual(TestObjectError, 449, "TestObjectError")

    ' tdf#151503 - error handling of missing optional parameters (boolean operations)
    ' Without the fix in place, this test would have failed with:
    ' - Expected: 449 (ERRCODE_BASIC_NOT_OPTIONAL - Argument not optional)
    ' - Actual  : 0 (No error code since a missing parameter evaluates to true)
    TestUtil.AssertEqual(TestBooleanOperations, 449, "TestBooleanOperations")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testOptionalsCompatible", Err, Error$, Erl)
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

Function TestArithmeticOperator(Optional optInt)
On Error GoTo errorHandler
    optInt = optInt + 100
    TestArithmeticOperator = optInt
errorHandler:
    TestArithmeticOperator = Err()
End Function

Function TestUnaryOperator(Optional optInt)
On Error GoTo errorHandler
    If (Not optInt) Then optInt = 100
    TestUnaryOperator = optInt
errorHandler:
    TestUnaryOperator = Err()
End Function

Function TestCollection(Optional optInt)
On Error GoTo errorHandler
    Dim cA As New Collection
    cA.Add(optInt)
    TestCollection = cA.Item(1) + 100
errorHandler:
    TestCollection = Err()
End Function

Function TestObjectError(Optional optInt)
On Error GoTo errorHandler
    Dim aTestObject As Variant
    aTestObject = CreateObject("testObject")
    aTestObject.testInt = optInt
    TestObjectError = optInt
errorHandler:
    TestObjectError = Err()
End Function

Function TestBooleanOperations(Optional optBool As Boolean)
On Error GoTo errorHandler
    if optBool then
        TestBooleanOperations = 0
    end if
errorHandler:
    TestBooleanOperations = Err()
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
