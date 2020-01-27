Option VBASupport 1

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Const IsMissingNone = -1
Const IsMissingA = 0
Const IsMissingB = 1
Const IsMissingAB = 2

Function doUnitTest() As String
    result = verify_testIsMissingVba()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = result
    Else
        doUnitTest = "OK"
    End If
End Function

' tdf#36737 - Test isMissing function with different datatypes. In LO Basic
' with option VBASupport, optional parameters are allowed including additional
' default values. Missing optional parameters having types other than variant,
' which don't have explicit default values, will be initialized to their
' default value of its datatype.
Function verify_testIsMissingVba() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)
    testName = "Test optionals (Basic)"
    On Error GoTo errorHandler

    ' optionals with variant datatypes
    TestLog_ASSERT TestOptVariant(), IsMissingA, "TestOptVariant()"
    TestLog_ASSERT TestOptVariant(123), IsMissingNone, "TestOptVariant(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptVariant(, 456), IsMissingNone, "TestOptVariant(, 456)"
    TestLog_ASSERT TestOptVariant(123, 456), IsMissingNone, "TestOptVariant(123, 456)"

    ' optionals with variant datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptVariantByRefByVal(), IsMissingA, "TestOptVariantByRefByVal()"
    TestLog_ASSERT TestOptVariantByRefByVal(123),IsMissingNone, "TestOptVariantByRefByVal(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptVariantByRefByVal(, 456), IsMissingNone, "TestOptVariantByRefByVal(, 456)"
    TestLog_ASSERT TestOptVariantByRefByVal(123, 456), IsMissingNone, "TestOptVariantByRefByVal(123, 456)"

    ' optionals with double datatypes
    TestLog_ASSERT TestOptDouble(), IsMissingNone, "TestOptDouble()"
    TestLog_ASSERT TestOptDouble(123.4), IsMissingNone, "TestOptDouble(123.4)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptDouble(, 567.8), IsMissingNone, "TestOptDouble(, 567.8)"
    TestLog_ASSERT TestOptDouble(123.4, 567.8), IsMissingNone, "TestOptDouble(123.4, 567.8)"

    ' optionals with double datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptDoubleByRefByVal(), IsMissingNone, "TestOptDouble()"
    TestLog_ASSERT TestOptDoubleByRefByVal(123.4), IsMissingNone, "TestOptDouble(123.4)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptDoubleByRefByVal(, 567.8), IsMissingNone, "TestOptDoubleByRefByVal(, 567.8)"
    TestLog_ASSERT TestOptDoubleByRefByVal(123.4, 567.8), IsMissingNone, "TestOptDoubleByRefByVal(123.4, 567.8)"

    ' optionals with integer datatypes
    TestLog_ASSERT TestOptInteger(), IsMissingNone, "TestOptInteger()"
    TestLog_ASSERT TestOptInteger(123), IsMissingNone, "TestOptInteger(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptInteger(, 456), IsMissingNone, "TestOptInteger(, 456)"
    TestLog_ASSERT TestOptInteger(123, 456), IsMissingNone, "TestOptInteger(123, 456)"

    ' optionals with integer datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptIntegerByRefByVal(), IsMissingNone, "TestOptIntegerByRefByVal()"
    TestLog_ASSERT TestOptIntegerByRefByVal(123), IsMissingNone, "TestOptIntegerByRefByVal(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptIntegerByRefByVal(, 456), IsMissingNone, "TestOptIntegerByRefByVal(, 456)"
    TestLog_ASSERT TestOptIntegerByRefByVal(123, 456), IsMissingNone, "TestOptIntegerByRefByVal(123, 456)"

    ' optionals with string datatypes
    TestLog_ASSERT TestOptString(), IsMissingNone, "TestOptString()"
    TestLog_ASSERT TestOptString("123"), IsMissingNone, "TestOptString(""123"")"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptString(, "456"), IsMissingNone, "TestOptString(, ""456"")"
    TestLog_ASSERT TestOptString("123", "456"), IsMissingNone, "TestOptString(""123"", ""456"")"

    ' optionals with string datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptStringByRefByVal(), IsMissingNone, "TestOptStringByRefByVal()"
    TestLog_ASSERT TestOptStringByRefByVal("123"), IsMissingNone, "TestOptStringByRefByVal(""123"")"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptStringByRefByVal(, "456"), IsMissingNone, "TestOptStringByRefByVal(, ""456"")"
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
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptObject(, cB), IsMissingA, "TestOptObject(, B)"
    TestLog_ASSERT TestOptObject(cA, cB), IsMissingNone, "TestOptObject(A, B)"

    ' optionals with object datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptObjectByRefByVal(), IsMissingAB, "TestOptObjectByRefByVal()"
    TestLog_ASSERT TestOptObjectByRefByVal(cA), IsMissingB, "TestOptObjectByRefByVal(A)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptObjectByRefByVal(, cB), IsMissingA, "TestOptObjectByRefByVal(, B)"
    TestLog_ASSERT TestOptObjectByRefByVal(cA, cB), IsMissingNone, "TestOptObjectByRefByVal(A, B)"

    ' optionals with array datatypes
    Dim aA(0 To 1) As Integer
    aA(0) = 123
    aA(1) = 456
    Dim aB(0 To 1) As Variant
    aB(0) = 123.4
    aB(1) = 567.8
    ' TODO - New bug report? Scanner initializes variable not as an array
    ' TestLog_ASSERT TestOptArray(), IsMissingAB, "TestOptArray()"
    ' TestLog_ASSERT TestOptArray(aA), IsMissingB, "TestOptArray(A)"

    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptArray(, aB), IsMissingA, "TestOptArray(, B)"
    TestLog_ASSERT TestOptArray(aA, aB), IsMissingNone, "TestOptArray(A, B)"

    ' optionals with array datatypes (ByRef and ByVal)
    ' TODO - New bug report? Scanner initializes variable not as an array
    ' TestLog_ASSERT TestOptArrayByRefByVal(), IsMissingAB, "TestOptArrayByRefByVal()"
    ' TestLog_ASSERT TestOptArrayByRefByVal(aA), IsMissingB, "TestOptArrayByRefByVal(A)"

    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptArrayByRefByVal(, aB), IsMissingA, "TestOptArrayByRefByVal(, B)"
    TestLog_ASSERT TestOptArrayByRefByVal(aA, aB), IsMissingNone, "TestOptArrayByRefByVal(A, B)"

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testIsMissingVba = result

    Exit Function
errorHandler:
    TestLog_ASSERT False, testName & ": hit error handler"
End Function

Function TestOptVariant(Optional A, Optional B As Variant = 123)
    If IsMissing(A) And IsMissing(B) Then
        TestOptVariant = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptVariant = IsMissingA
    ElseIf IsMissing(B) Then
        TestOptVariant = IsMissingB
    Else
        TestOptVariant = IsMissingNone
    End If
End Function

Function TestOptVariantByRefByVal(Optional ByRef A, Optional ByVal B As Variant = 123)
    If IsMissing(A) And IsMissing(B) Then
        TestOptVariantByRefByVal = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptVariantByRefByVal = IsMissingA
    ElseIf IsMissing(B) Then
        TestOptVariantByRefByVal = IsMissingB
    Else
        TestOptVariantByRefByVal = IsMissingNone
    End If
End Function


Function TestOptDouble(Optional A As Double, Optional B As Double = 123.4)
    If IsMissing(A) And IsMissing(B) Then
        TestOptDouble = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptDouble = IsMissingA
    ElseIf IsMissing(B) Then
        TestOptDouble = IsMissingB
    Else
        TestOptDouble = IsMissingNone
    End If
End Function

Function TestOptDoubleByRefByVal(Optional ByRef A As Double, Optional ByVal B As Double = 123.4)
    If IsMissing(A) And IsMissing(B) Then
        TestOptDoubleByRefByVal = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptDoubleByRefByVal = IsMissingA
    ElseIf IsMissing(B) Then
        TestOptDoubleByRefByVal = IsMissingB
    Else
        TestOptDoubleByRefByVal = IsMissingNone
    End If
End Function

Function TestOptInteger(Optional A As Integer, Optional B As Integer = 123)
    If IsMissing(A) And IsMissing(B) Then
        TestOptInteger = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptInteger = IsMissingA
    ElseIf IsMissing(B) Then
        TestOptInteger = IsMissingB
    Else
        TestOptInteger = IsMissingNone
    End If
End Function

Function TestOptIntegerByRefByVal(Optional ByRef A As Integer, Optional ByVal B As Integer = 123)
    If IsMissing(A) And IsMissing(B) Then
        TestOptIntegerByRefByVal = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptIntegerByRefByVal = IsMissingA
    ElseIf IsMissing(B) Then
        TestOptIntegerByRefByVal = IsMissingB
    Else
        TestOptIntegerByRefByVal = IsMissingNone
    End If
End Function

Function TestOptString(Optional A As String, Optional B As String = "123")
    If IsMissing(A) And IsMissing(B) Then
        TestOptString = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptString = IsMissingA
    ElseIf IsMissing(B) Then
        TestOptString = IsMissingB
    Else
        TestOptString = IsMissingNone
    End If
End Function

Function TestOptStringByRefByVal(Optional ByRef A As String, Optional ByVal B As String = "123")
    If IsMissing(A) And IsMissing(B) Then
        TestOptStringByRefByVal = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptStringByRefByVal = IsMissingA
    ElseIf IsMissing(B) Then
        TestOptStringByRefByVal = IsMissingB
    Else
        TestOptStringByRefByVal = IsMissingNone
    End If
End Function

Function TestOptObject(Optional A As Collection, Optional B As Collection)
    Dim idx As Integer
    If IsNull(A) And IsNull(B) Then
        TestOptObject = IsMissingAB
    ElseIf IsNull(A) Then
        TestOptObject = IsMissingA
    ElseIf IsNull(B) Then
        TestOptObject = IsMissingB
    Else
        TestOptObject = IsMissingNone
    End If
End Function

Function TestOptObjectByRefByVal(Optional ByRef A As Collection, Optional ByVal B As Collection)
    Dim idx As Integer
    If IsNull(A) And IsNull(B) Then
        TestOptObjectByRefByVal = IsMissingAB
    ElseIf IsNull(A) Then
        TestOptObjectByRefByVal = IsMissingA
    ElseIf IsNull(B) Then
        TestOptObjectByRefByVal = IsMissingB
    Else
        TestOptObjectByRefByVal = IsMissingNone
    End If
End Function

Function TestOptArray(Optional A() As Integer, Optional B() As Variant)
    Dim idx As Integer
    If IsEmpty(A) And IsEmpty(B) Then
        TestOptArray = IsMissingAB
    ElseIf IsEmpty(A) Then
        TestOptArray = IsMissingA
    ElseIf IsEmpty(B) Then
        TestOptArray = IsMissingB
    Else
        TestOptArray = IsMissingNone
    End If
End Function

Function TestOptArrayByRefByVal(Optional ByRef A() As Integer, Optional ByVal B() As Variant)
    Dim idx As Integer
    If IsEmpty(A) And IsEmpty(B) Then
        TestOptArrayByRefByVal = IsMissingAB
    ElseIf IsEmpty(A) Then
        TestOptArrayByRefByVal = IsMissingA
    ElseIf IsEmpty(B) Then
        TestOptArrayByRefByVal = IsMissingB
    Else
        TestOptArrayByRefByVal = IsMissingNone
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