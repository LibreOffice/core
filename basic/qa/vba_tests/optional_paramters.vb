'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testOptionalsVba
    doUnitTest = TestUtil.GetResult()
End Function

' tdf#36737 - Test optionals with different datatypes. In LO Basic
' with option VBASupport, optional parameters are allowed including additional
' default values. Missing optional parameters having types other than variant,
' which don't have explicit default values, will be initialized to their
' respective default value of its datatype
Sub verify_testOptionalsVba()
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
    TestUtil.AssertEqual(TestOptDouble(), 123.4, "TestOptDouble()")
    TestUtil.AssertEqual(TestOptDouble(123.4), 246.8, "TestOptDouble(123.4)")
    TestUtil.AssertEqual(TestOptDouble(, 567.8), 567.8, "TestOptDouble(, 567.8)")
    TestUtil.AssertEqual(Format(TestOptDouble(123.4, 567.8), "0.0"), 691.2, "TestOptDouble(123.4, 567.8)")

    ' optionals with double datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptDoubleByRefByVal(), 123.4, "TestOptDouble()")
    TestUtil.AssertEqual(TestOptDoubleByRefByVal(123.4), 246.8, "TestOptDouble(123.4)")
    TestUtil.AssertEqual(TestOptDoubleByRefByVal(, 567.8), 567.8, "TestOptDoubleByRefByVal(, 567.8)")
    TestUtil.AssertEqual(Format(TestOptDoubleByRefByVal(123.4, 567.8), "0.0"), 691.2, "TestOptDoubleByRefByVal(123.4, 567.8)")

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
    TestUtil.AssertEqual(Format(TestOptObject(, cB), "0.0"), 691.2, "TestOptObject(, B)")
    TestUtil.AssertEqual(Format(TestOptObject(cA, cB), "0.0"), 1270.2, "TestOptObject(A, B)")

    ' optionals with object datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptObjectByRefByVal(), 0, "TestOptObjectByRefByVal()")
    TestUtil.AssertEqual(TestOptObjectByRefByVal(cA), 579, "TestOptObjectByRefByVal(A)")
    TestUtil.AssertEqual(Format(TestOptObjectByRefByVal(, cB), "0.0"), 691.2, "TestOptObjectByRefByVal(, B)")
    TestUtil.AssertEqual(Format(TestOptObjectByRefByVal(cA, cB), "0.0"), 1270.2, "TestOptObjectByRefByVal(A, B)")

    ' optionals with array datatypes
    Dim aA(0 To 1) As Integer
    aA(0) = 123
    aA(1) = 456
    Dim aB(0 To 1) As Variant
    aB(0) = 123.4
    aB(1) = 567.8
    ' TODO - New bug report? Scanner initializes variable not as an array
    ' TestUtil.AssertEqual(TestOptArray(), 0, "TestOptArray()")
    ' TestUtil.AssertEqual(TestOptArray(aA), 579, "TestOptArray(A)")
    ' TestUtil.AssertEqual(Format(TestOptArray(, aB), "0.0"), 691.2, "TestOptArray(, B)")
    TestUtil.AssertEqual(Format(TestOptArray(aA, aB), "0.0"), 1270.2, "TestOptArray(A, B)")

    ' optionals with array datatypes (ByRef and ByVal)
    ' TODO - New bug report? Scanner initializes variable not as an array
    ' TestUtil.AssertEqual(TestOptArrayByRefByVal(), 0, "TestOptArrayByRefByVal()")
    ' TestUtil.AssertEqual(TestOptArrayByRefByVal(aA), 579, "TestOptArrayByRefByVal(A)")
    ' TestUtil.AssertEqual(Format(TestOptArrayByRefByVal(, aB), "0.0"), 691.2, "TestOptArrayByRefByVal(, B)")
    TestUtil.AssertEqual(Format(TestOptArrayByRefByVal(aA, aB), "0.0"), 1270.2, "TestOptArrayByRefByVal(A, B)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testOptionalsVba", Err, Error$, Erl)
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
