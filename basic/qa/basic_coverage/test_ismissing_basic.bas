' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Const IsMissingNone = -1
Const IsMissingA = 0
Const IsMissingB = 1
Const IsMissingAB = 2

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testIsMissingBasic
    doUnitTest = TestUtil.GetResult()
End Function

' tdf#36737 - Test optionals with different datatypes. In LO Basic, optional
' parameters are allowed, but without any default values. Missing optional parameters
' will not be initialized to their respective default values of its datatype, either.
Sub verify_testIsMissingBasic()

    On Error GoTo errorHandler

    ' optionals with variant datatypes
    TestUtil.AssertEqual(TestOptVariant(), IsMissingAB, "TestOptVariant()")
    TestUtil.AssertEqual(TestOptVariant(123), IsMissingB, "TestOptVariant(123)")
    TestUtil.AssertEqual(TestOptVariant(, 456), IsMissingA, "TestOptVariant(, 456)")
    TestUtil.AssertEqual(TestOptVariant(123, 456), IsMissingNone, "TestOptVariant(123, 456)")

    ' optionals with variant datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptVariantByRefByVal(), IsMissingAB, "TestOptVariantByRefByVal()")
    TestUtil.AssertEqual(TestOptVariantByRefByVal(123), IsMissingB, "TestOptVariantByRefByVal(123)")
    TestUtil.AssertEqual(TestOptVariantByRefByVal(, 456), IsMissingA, "TestOptVariantByRefByVal(, 456)")
    TestUtil.AssertEqual(TestOptVariantByRefByVal(123, 456), IsMissingNone, "TestOptVariantByRefByVal(123, 456)")

    ' optionals with double datatypes
    TestUtil.AssertEqual(TestOptDouble(), IsMissingAB, "TestOptDouble()")
    TestUtil.AssertEqual(TestOptDouble(123.4), IsMissingB, "TestOptDouble(123.4)")
    TestUtil.AssertEqual(TestOptDouble(, 567.8), IsMissingA, "TestOptDouble(, 567.8)")
    TestUtil.AssertEqual(TestOptDouble(123.4, 567.8), IsMissingNone, "TestOptDouble(123.4, 567.8)")

    ' optionals with double datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptDoubleByRefByVal(), IsMissingAB, "TestOptDouble()")
    TestUtil.AssertEqual(TestOptDoubleByRefByVal(123.4), IsMissingB, "TestOptDouble(123.4)")
    TestUtil.AssertEqual(TestOptDoubleByRefByVal(, 567.8), IsMissingA, "TestOptDoubleByRefByVal(, 567.8)")
    TestUtil.AssertEqual(TestOptDoubleByRefByVal(123.4, 567.8), IsMissingNone, "TestOptDoubleByRefByVal(123.4, 567.8)")

    ' optionals with integer datatypes
    TestUtil.AssertEqual(TestOptInteger(), IsMissingAB, "TestOptInteger()")
    TestUtil.AssertEqual(TestOptInteger(123), IsMissingB, "TestOptInteger(123)")
    TestUtil.AssertEqual(TestOptInteger(, 456), IsMissingA, "TestOptInteger(, 456)")
    TestUtil.AssertEqual(TestOptInteger(123, 456), IsMissingNone, "TestOptInteger(123, 456)")

    ' optionals with integer datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(), IsMissingAB, "TestOptIntegerByRefByVal()")
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(123), IsMissingB, "TestOptIntegerByRefByVal(123)")
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(, 456), IsMissingA, "TestOptIntegerByRefByVal(, 456)")
    TestUtil.AssertEqual(TestOptIntegerByRefByVal(123, 456), IsMissingNone, "TestOptIntegerByRefByVal(123, 456)")

    ' optionals with string datatypes
    TestUtil.AssertEqual(TestOptString(), IsMissingAB, "TestOptString()")
    TestUtil.AssertEqual(TestOptString("123"), IsMissingB, "TestOptString(""123"")")
    TestUtil.AssertEqual(TestOptString(, "456"), IsMissingA, "TestOptString(, ""456"")")
    TestUtil.AssertEqual(TestOptString("123", "456"), IsMissingNone, "TestOptString(""123"", ""456"")")

    ' optionals with string datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptStringByRefByVal(), IsMissingAB, "TestOptStringByRefByVal()")
    TestUtil.AssertEqual(TestOptStringByRefByVal("123"), IsMissingB, "TestOptStringByRefByVal(""123"")")
    TestUtil.AssertEqual(TestOptStringByRefByVal(, "456"), IsMissingA, "TestOptStringByRefByVal(, ""456"")")
    TestUtil.AssertEqual(TestOptStringByRefByVal("123", "456"), IsMissingNone, "TestOptStringByRefByVal(""123"", ""456"")")

    ' optionals with object datatypes
    Dim cA As New Collection
    cA.Add (123)
    cA.Add (456)
    Dim cB As New Collection
    cB.Add (123.4)
    cB.Add (567.8)
    TestUtil.AssertEqual(TestOptObject(), IsMissingAB, "TestOptObject()")
    TestUtil.AssertEqual(TestOptObject(cA), IsMissingB, "TestOptObject(A)")
    TestUtil.AssertEqual(TestOptObject(, cB), IsMissingA, "TestOptObject(, B)")
    TestUtil.AssertEqual(TestOptObject(cA, cB), IsMissingNone, "TestOptObject(A, B)")

    ' optionals with object datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptObjectByRefByVal(), IsMissingAB, "TestOptObjectByRefByVal()")
    TestUtil.AssertEqual(TestOptObjectByRefByVal(cA), IsMissingB, "TestOptObjectByRefByVal(A)")
    TestUtil.AssertEqual(TestOptObjectByRefByVal(, cB), IsMissingA, "TestOptObjectByRefByVal(, B)")
    TestUtil.AssertEqual(TestOptObjectByRefByVal(cA, cB), IsMissingNone, "TestOptObjectByRefByVal(A, B)")

    ' optionals with array datatypes
    Dim aA(0 To 1) As Integer
    aA(0) = 123
    aA(1) = 456
    Dim aB(0 To 1) As Variant
    aB(0) = 123.4
    aB(1) = 567.8
    TestUtil.AssertEqual(TestOptArray(), IsMissingAB, "TestOptArray()")
    TestUtil.AssertEqual(TestOptArray(aA), IsMissingB, "TestOptArray(A)")
    TestUtil.AssertEqual(TestOptArray(, aB), IsMissingA, "TestOptArray(, B)")
    TestUtil.AssertEqual(TestOptArray(aA, aB), IsMissingNone, "TestOptArray(A, B)")

    ' optionals with array datatypes (ByRef and ByVal)
    TestUtil.AssertEqual(TestOptArrayByRefByVal(), IsMissingAB, "TestOptArrayByRefByVal()")
    TestUtil.AssertEqual(TestOptArrayByRefByVal(aA), IsMissingB, "TestOptArrayByRefByVal(A)")
    TestUtil.AssertEqual(TestOptArrayByRefByVal(, aB), IsMissingA, "TestOptArrayByRefByVal(, B)")
    TestUtil.AssertEqual(TestOptArrayByRefByVal(aA, aB), IsMissingNone, "TestOptArrayByRefByVal(A, B)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testIsMissingBasic", Err, Error$, Erl)
End Sub

Function TestOptVariant(Optional A, Optional B As Variant)
    TestOptVariant = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptVariantByRefByVal(Optional ByRef A, Optional ByVal B As Variant)
    TestOptVariantByRefByVal = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptDouble(Optional A As Double, Optional B As Double)
    TestOptDouble = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptDoubleByRefByVal(Optional ByRef A As Double, Optional ByVal B As Double)
    TestOptDoubleByRefByVal = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptInteger(Optional A As Integer, Optional B As Integer)
    TestOptInteger = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptIntegerByRefByVal(Optional ByRef A As Integer, Optional ByVal B As Integer)
    TestOptIntegerByRefByVal = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptString(Optional A As String, Optional B As String)
    TestOptString = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptStringByRefByVal(Optional ByRef A As String, Optional ByVal B As String)
    TestOptStringByRefByVal = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptObject(Optional A As Collection, Optional B As Collection)
    TestOptObject = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptObjectByRefByVal(Optional ByRef A As Collection, Optional ByVal B As Collection)
    TestOptObjectByRefByVal = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptArray(Optional A() As Integer, Optional B() As Variant)
    TestOptArray = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptArrayByRefByVal(Optional ByRef A() As Integer, Optional ByVal B() As Variant)
    TestOptArrayByRefByVal = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function WhatIsMissing(is_missingA, is_missingB)
    If is_missingA And is_missingB Then
        WhatIsMissing = IsMissingAB
    ElseIf is_missingA Then
        WhatIsMissing = IsMissingA
    ElseIf is_missingB Then
        WhatIsMissing = IsMissingB
    Else
        WhatIsMissing = IsMissingNone
    End If
End Function
