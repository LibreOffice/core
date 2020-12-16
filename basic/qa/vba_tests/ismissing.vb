Option VBASupport 1

Const IsMissingNone = -1
Const IsMissingA = 0
Const IsMissingB = 1
Const IsMissingAB = 2

Function doUnitTest() As String
    verify_testIsMissingVba
    doUnitTest = TestUtilModule.GetResult()
End Function

' tdf#36737 - Test isMissing function with different datatypes. In LO Basic
' with option VBASupport, optional parameters are allowed including additional
' default values. Missing optional parameters having types other than variant,
' which don't have explicit default values, will be initialized to their
' respective default value of its datatype.
Sub verify_testIsMissingVba()

    TestUtilModule.TestInit
    testName = "Test missing (VBA)"
    On Error GoTo errorHandler

    ' optionals with variant datatypes
    TestUtilModule.AssertEqual(TestOptVariant(), IsMissingA, "TestOptVariant()")
    TestUtilModule.AssertEqual(TestOptVariant(123), IsMissingNone, "TestOptVariant(123)")
    TestUtilModule.AssertEqual(TestOptVariant(, 456), IsMissingA, "TestOptVariant(, 456)")
    TestUtilModule.AssertEqual(TestOptVariant(123, 456), IsMissingNone, "TestOptVariant(123, 456)")

    ' optionals with variant datatypes (ByRef and ByVal)
    TestUtilModule.AssertEqual(TestOptVariantByRefByVal(), IsMissingA, "TestOptVariantByRefByVal()")
    TestUtilModule.AssertEqual(TestOptVariantByRefByVal(123),IsMissingNone, "TestOptVariantByRefByVal(123)")
    TestUtilModule.AssertEqual(TestOptVariantByRefByVal(, 456), IsMissingA, "TestOptVariantByRefByVal(, 456)")
    TestUtilModule.AssertEqual(TestOptVariantByRefByVal(123, 456), IsMissingNone, "TestOptVariantByRefByVal(123, 456)")

    ' optionals with double datatypes
    TestUtilModule.AssertEqual(TestOptDouble(), IsMissingNone, "TestOptDouble()")
    TestUtilModule.AssertEqual(TestOptDouble(123.4), IsMissingNone, "TestOptDouble(123.4)")
    TestUtilModule.AssertEqual(TestOptDouble(, 567.8), IsMissingNone, "TestOptDouble(, 567.8)")
    TestUtilModule.AssertEqual(TestOptDouble(123.4, 567.8), IsMissingNone, "TestOptDouble(123.4, 567.8)")

    ' optionals with double datatypes (ByRef and ByVal)
    TestUtilModule.AssertEqual(TestOptDoubleByRefByVal(), IsMissingNone, "TestOptDouble()")
    TestUtilModule.AssertEqual(TestOptDoubleByRefByVal(123.4), IsMissingNone, "TestOptDouble(123.4)")
    TestUtilModule.AssertEqual(TestOptDoubleByRefByVal(, 567.8), IsMissingNone, "TestOptDoubleByRefByVal(, 567.8)")
    TestUtilModule.AssertEqual(TestOptDoubleByRefByVal(123.4, 567.8), IsMissingNone, "TestOptDoubleByRefByVal(123.4, 567.8)")

    ' optionals with integer datatypes
    TestUtilModule.AssertEqual(TestOptInteger(), IsMissingNone, "TestOptInteger()")
    TestUtilModule.AssertEqual(TestOptInteger(123), IsMissingNone, "TestOptInteger(123)")
    TestUtilModule.AssertEqual(TestOptInteger(, 456), IsMissingNone, "TestOptInteger(, 456)")
    TestUtilModule.AssertEqual(TestOptInteger(123, 456), IsMissingNone, "TestOptInteger(123, 456)")

    ' optionals with integer datatypes (ByRef and ByVal)
    TestUtilModule.AssertEqual(TestOptIntegerByRefByVal(), IsMissingNone, "TestOptIntegerByRefByVal()")
    TestUtilModule.AssertEqual(TestOptIntegerByRefByVal(123), IsMissingNone, "TestOptIntegerByRefByVal(123)")
    TestUtilModule.AssertEqual(TestOptIntegerByRefByVal(, 456), IsMissingNone, "TestOptIntegerByRefByVal(, 456)")
    TestUtilModule.AssertEqual(TestOptIntegerByRefByVal(123, 456), IsMissingNone, "TestOptIntegerByRefByVal(123, 456)")

    ' optionals with string datatypes
    TestUtilModule.AssertEqual(TestOptString(), IsMissingNone, "TestOptString()")
    TestUtilModule.AssertEqual(TestOptString("123"), IsMissingNone, "TestOptString(""123"")")
    TestUtilModule.AssertEqual(TestOptString(, "456"), IsMissingNone, "TestOptString(, ""456"")")
    TestUtilModule.AssertEqual(TestOptString("123", "456"), IsMissingNone, "TestOptString(""123"", ""456"")")

    ' optionals with string datatypes (ByRef and ByVal)
    TestUtilModule.AssertEqual(TestOptStringByRefByVal(), IsMissingNone, "TestOptStringByRefByVal()")
    TestUtilModule.AssertEqual(TestOptStringByRefByVal("123"), IsMissingNone, "TestOptStringByRefByVal(""123"")")
    TestUtilModule.AssertEqual(TestOptStringByRefByVal(, "456"), IsMissingNone, "TestOptStringByRefByVal(, ""456"")")
    TestUtilModule.AssertEqual(TestOptStringByRefByVal("123", "456"), IsMissingNone, "TestOptStringByRefByVal(""123"", ""456"")")

    ' optionals with object datatypes
    Dim cA As New Collection
    cA.Add (123)
    cA.Add (456)
    Dim cB As New Collection
    cB.Add (123.4)
    cB.Add (567.8)
    TestUtilModule.AssertEqual(TestOptObject(), IsMissingAB, "TestOptObject()")
    TestUtilModule.AssertEqual(TestOptObject(cA), IsMissingB, "TestOptObject(A)")
    TestUtilModule.AssertEqual(TestOptObject(, cB), IsMissingA, "TestOptObject(, B)")
    TestUtilModule.AssertEqual(TestOptObject(cA, cB), IsMissingNone, "TestOptObject(A, B)")

    ' optionals with object datatypes (ByRef and ByVal)
    TestUtilModule.AssertEqual(TestOptObjectByRefByVal(), IsMissingAB, "TestOptObjectByRefByVal()")
    TestUtilModule.AssertEqual(TestOptObjectByRefByVal(cA), IsMissingB, "TestOptObjectByRefByVal(A)")
    TestUtilModule.AssertEqual(TestOptObjectByRefByVal(, cB), IsMissingA, "TestOptObjectByRefByVal(, B)")
    TestUtilModule.AssertEqual(TestOptObjectByRefByVal(cA, cB), IsMissingNone, "TestOptObjectByRefByVal(A, B)")

    ' optionals with array datatypes
    Dim aA(0 To 1) As Integer
    aA(0) = 123
    aA(1) = 456
    Dim aB(0 To 1) As Variant
    aB(0) = 123.4
    aB(1) = 567.8
    ' TODO - New bug report? Scanner initializes variable not as an array
    ' TestUtilModule.AssertEqual(TestOptArray(), IsMissingAB, "TestOptArray()")
    ' TestUtilModule.AssertEqual(TestOptArray(aA), IsMissingB, "TestOptArray(A)")
    ' TestUtilModule.AssertEqual(TestOptArray(, aB), IsMissingA, "TestOptArray(, B)")
    TestUtilModule.AssertEqual(TestOptArray(aA, aB), IsMissingNone, "TestOptArray(A, B)")

    ' optionals with array datatypes (ByRef and ByVal)
    ' TODO - New bug report? Scanner initializes variable not as an array
    ' TestUtilModule.AssertEqual(TestOptArrayByRefByVal(), IsMissingAB, "TestOptArrayByRefByVal()")
    ' TestUtilModule.AssertEqual(TestOptArrayByRefByVal(aA), IsMissingB, "TestOptArrayByRefByVal(A)")
    ' TestUtilModule.AssertEqual(TestOptArrayByRefByVal(, aB), IsMissingA, "TestOptArrayByRefByVal(, B)")
    TestUtilModule.AssertEqual(TestOptArrayByRefByVal(aA, aB), IsMissingNone, "TestOptArrayByRefByVal(A, B)")

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
    TestUtilModule.AssertEqual(False, True, Err.Description)
End Sub

Function TestOptVariant(Optional A, Optional B As Variant = 123)
    TestOptVariant = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptVariantByRefByVal(Optional ByRef A, Optional ByVal B As Variant = 123)
    TestOptVariantByRefByVal = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptDouble(Optional A As Double, Optional B As Double = 123.4)
    TestOptDouble = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptDoubleByRefByVal(Optional ByRef A As Double, Optional ByVal B As Double = 123.4)
    TestOptDoubleByRefByVal = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptInteger(Optional A As Integer, Optional B As Integer = 123)
    TestOptInteger = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptIntegerByRefByVal(Optional ByRef A As Integer, Optional ByVal B As Integer = 123)
    TestOptIntegerByRefByVal = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptString(Optional A As String, Optional B As String = "123")
    TestOptString = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptStringByRefByVal(Optional ByRef A As String, Optional ByVal B As String = "123")
    TestOptStringByRefByVal = WhatIsMissing(IsMissing(A), IsMissing(B))
End Function

Function TestOptObject(Optional A As Collection, Optional B As Collection)
    TestOptObject = WhatIsMissing(IsNull(A), IsNull(B))
End Function

Function TestOptObjectByRefByVal(Optional ByRef A As Collection, Optional ByVal B As Collection)
    TestOptObjectByRefByVal = WhatIsMissing(IsNull(A), IsNull(B))
End Function

Function TestOptArray(Optional A() As Integer, Optional B() As Variant)
    TestOptArray = WhatIsMissing(IsEmpty(A), IsEmpty(B))
End Function

Function TestOptArrayByRefByVal(Optional ByRef A() As Integer, Optional ByVal B() As Variant)
    TestOptArrayByRefByVal = WhatIsMissing(IsEmpty(A), IsEmpty(B))
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

