Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Const IsMissingNone = -1
Const IsMissingA = 0
Const IsMissingB = 1
Const IsMissingAB = 2

Function doUnitTest() As String
    result = verify_testIsMissingBasic()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function

' tdf#36737 - Test optionals with different datatypes. In LO Basic, optional
' parameters are allowed, but without any default values. Missing optional parameters
' will not be initialized to their respective default values of its datatype, either.
Function verify_testIsMissingBasic() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)
    testName = "Test missing (Basic)"
    On Error GoTo errorHandler

    ' optionals with variant datatypes
    TestLog_ASSERT TestOptVariant(), IsMissingAB, "TestOptVariant()"
    TestLog_ASSERT TestOptVariant(123), IsMissingB, "TestOptVariant(123)"
    TestLog_ASSERT TestOptVariant(, 456), IsMissingA, "TestOptVariant(, 456)"
    TestLog_ASSERT TestOptVariant(123, 456), IsMissingNone, "TestOptVariant(123, 456)"

    ' optionals with variant datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptVariantByRefByVal(), IsMissingAB, "TestOptVariantByRefByVal()"
    TestLog_ASSERT TestOptVariantByRefByVal(123), IsMissingB, "TestOptVariantByRefByVal(123)"
    TestLog_ASSERT TestOptVariantByRefByVal(, 456), IsMissingA, "TestOptVariantByRefByVal(, 456)"
    TestLog_ASSERT TestOptVariantByRefByVal(123, 456), IsMissingNone, "TestOptVariantByRefByVal(123, 456)"

    ' optionals with double datatypes
    TestLog_ASSERT TestOptDouble(), IsMissingAB, "TestOptDouble()"
    TestLog_ASSERT TestOptDouble(123.4), IsMissingB, "TestOptDouble(123.4)"
    TestLog_ASSERT TestOptDouble(, 567.8), IsMissingA, "TestOptDouble(, 567.8)"
    TestLog_ASSERT TestOptDouble(123.4, 567.8), IsMissingNone, "TestOptDouble(123.4, 567.8)"

    ' optionals with double datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptDoubleByRefByVal(), IsMissingAB, "TestOptDouble()"
    TestLog_ASSERT TestOptDoubleByRefByVal(123.4), IsMissingB, "TestOptDouble(123.4)"
    TestLog_ASSERT TestOptDoubleByRefByVal(, 567.8), IsMissingA, "TestOptDoubleByRefByVal(, 567.8)"
    TestLog_ASSERT TestOptDoubleByRefByVal(123.4, 567.8), IsMissingNone, "TestOptDoubleByRefByVal(123.4, 567.8)"

    ' optionals with integer datatypes
    TestLog_ASSERT TestOptInteger(), IsMissingAB, "TestOptInteger()"
    TestLog_ASSERT TestOptInteger(123), IsMissingB, "TestOptInteger(123)"
    TestLog_ASSERT TestOptInteger(, 456), IsMissingA, "TestOptInteger(, 456)"
    TestLog_ASSERT TestOptInteger(123, 456), IsMissingNone, "TestOptInteger(123, 456)"

    ' optionals with integer datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptIntegerByRefByVal(), IsMissingAB, "TestOptIntegerByRefByVal()"
    TestLog_ASSERT TestOptIntegerByRefByVal(123), IsMissingB, "TestOptIntegerByRefByVal(123)"
    TestLog_ASSERT TestOptIntegerByRefByVal(, 456), IsMissingA, "TestOptIntegerByRefByVal(, 456)"
    TestLog_ASSERT TestOptIntegerByRefByVal(123, 456), IsMissingNone, "TestOptIntegerByRefByVal(123, 456)"

    ' optionals with string datatypes
    TestLog_ASSERT TestOptString(), IsMissingAB, "TestOptString()"
    TestLog_ASSERT TestOptString("123"), IsMissingB, "TestOptString(""123"")"
    TestLog_ASSERT TestOptString(, "456"), IsMissingA, "TestOptString(, ""456"")"
    TestLog_ASSERT TestOptString("123", "456"), IsMissingNone, "TestOptString(""123"", ""456"")"

    ' optionals with string datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptStringByRefByVal(), IsMissingAB, "TestOptStringByRefByVal()"
    TestLog_ASSERT TestOptStringByRefByVal("123"), IsMissingB, "TestOptStringByRefByVal(""123"")"
    TestLog_ASSERT TestOptStringByRefByVal(, "456"), IsMissingA, "TestOptStringByRefByVal(, ""456"")"
    TestLog_ASSERT TestOptStringByRefByVal("123", "456"), IsMissingNone, "TestOptStringByRefByVal(""123"", ""456"")"

    ' optionals with object datatypes
    Dim cA As New Collection
    cA.Add (123)
    cA.Add (456)
    Dim cB As New Collection
    cB.Add (123.4)
    cB.Add (567.8)
    TestLog_ASSERT TestOptObject(), IsMissingAB, "TestOptObject()"
    TestLog_ASSERT TestOptObject(cA), IsMissingB, "TestOptObject(A)"
    TestLog_ASSERT TestOptObject(, cB), IsMissingA, "TestOptObject(, B)"
    TestLog_ASSERT TestOptObject(cA, cB), IsMissingNone, "TestOptObject(A, B)"

    ' optionals with object datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptObjectByRefByVal(), IsMissingAB, "TestOptObjectByRefByVal()"
    TestLog_ASSERT TestOptObjectByRefByVal(cA), IsMissingB, "TestOptObjectByRefByVal(A)"
    TestLog_ASSERT TestOptObjectByRefByVal(, cB), IsMissingA, "TestOptObjectByRefByVal(, B)"
    TestLog_ASSERT TestOptObjectByRefByVal(cA, cB), IsMissingNone, "TestOptObjectByRefByVal(A, B)"

    ' optionals with array datatypes
    Dim aA(0 To 1) As Integer
    aA(0) = 123
    aA(1) = 456
    Dim aB(0 To 1) As Variant
    aB(0) = 123.4
    aB(1) = 567.8
    TestLog_ASSERT TestOptArray(), IsMissingAB, "TestOptArray()"
    TestLog_ASSERT TestOptArray(aA), IsMissingB, "TestOptArray(A)"
    TestLog_ASSERT TestOptArray(, aB), IsMissingA, "TestOptArray(, B)"
    TestLog_ASSERT TestOptArray(aA, aB), IsMissingNone, "TestOptArray(A, B)"

    ' optionals with array datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptArrayByRefByVal(), IsMissingAB, "TestOptArrayByRefByVal()"
    TestLog_ASSERT TestOptArrayByRefByVal(aA), IsMissingB, "TestOptArrayByRefByVal(A)"
    TestLog_ASSERT TestOptArrayByRefByVal(, aB), IsMissingA, "TestOptArrayByRefByVal(, B)"
    TestLog_ASSERT TestOptArrayByRefByVal(aA, aB), IsMissingNone, "TestOptArrayByRefByVal(A, B)"

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testIsMissingBasic = result

    Exit Function
errorHandler:
    TestLog_ASSERT False, True, Err.Description
End Function

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

Sub TestLog_ASSERT(actual As Variant, expected As Integer, testName As String)
    If expected = actual Then
        passCount = passCount + 1
    Else
        result = result & Chr$(10) & " Failed: " & testName & " returned " & actual & ", expected " & expected
        failCount = failCount + 1
    End If
End Sub