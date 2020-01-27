Option Compatible

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Const IsMissingNone = -1
Const IsMissingA = 0
Const IsMissingB = 1
Const IsMissingAB = 2

Function doUnitTest() As String
    result = verify_testOptionalsBasic()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function

' tdf#36737 - Test optionals with different datatypes. In LO Basic
' with option Compatible, optional parameters are allowed with default values.
' Missing optional parameters will not be initialized to their default values
' of its datatype.
Function verify_testOptionalsBasic() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)
    testName = "Test optionals (Basic)"
    On Error GoTo errorHandler

    ' optionals with variant datatypes
    ' TODO - New bug report? Scanner initializes variable as String. Function returns "123"
    ' TestLog_ASSERT TestOptVariant() = 123, "the return of TestOptVariant(Optional, Optional) is: " & TestOptVariant(), "TestOptVariant()"
    TestLog_ASSERT TestOptVariant(123) = 246, "the return of TestOptVariant(Variant, Optional) is: " & TestOptVariant(123), "TestOptVariant(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptVariant(, 456) = 456, "the return of TestOptVariant(Optional, Variant) is: " & TestOptVariant(, 456), "TestOptVariant(, 456)"
    TestLog_ASSERT TestOptVariant(123, 456) = 579, "the return of TestOptVariant(Variant, Variant) is: " & TestOptVariant(123, 456), "TestOptVariant(123, 456)"

    ' optionals with variant datatypes (ByRef and ByVal)
    ' TODO - New bug report? Scanner initializes variable as String. Function returns "123"
    ' TestLog_ASSERT TestOptVariantByRefByVal() = 123, "the return of TestOptVariantByRefByVal(Optional, Optional) is: " & TestOptVariantByRefByVal(), "TestOptVariantByRefByVal()"
    TestLog_ASSERT TestOptVariantByRefByVal(123) = 246, "the return of TestOptVariantByRefByVal(Variant, Optional) is: " & TestOptVariantByRefByVal(123), "TestOptVariantByRefByVal(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptVariantByRefByVal(, 456) = 456, "the return of TestOptVariantByRefByVal(Optional, Variant) is: " & TestOptVariantByRefByVal(, 456), "TestOptVariantByRefByVal(, 456)"
    TestLog_ASSERT TestOptVariantByRefByVal(123, 456) = 579, "the return of TestOptVariantByRefByVal(Variant, Variant) is: " & TestOptVariantByRefByVal(123, 456), "TestOptVariantByRefByVal(123, 456)"

    ' optionals with double datatypes
    TestLog_ASSERT TestOptDouble() = 123.4, "the return of TestOptDouble(Optional, Optional) is: " & TestOptDouble(), "TestOptDouble()"
    TestLog_ASSERT TestOptDouble(123.4) = 246.8, "the return of TestOptDouble(Double, Optional) is: " & TestOptDouble(123.4), "TestOptDouble(123.4)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptDouble(, 567.8) = 567.8, "the return of TestOptDouble(Optional, Double) is: " & TestOptDouble(, 567.8), "TestOptDouble(, 567.8)"
    TestLog_ASSERT Format(TestOptDouble(123.4, 567.8), "0.0") = 691.2, "the return of TestOptDouble(Double, Double) is: " & TestOptDouble(123.4, 567.8), "TestOptDouble(123.4, 567.8)"

    ' optionals with double datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptDoubleByRefByVal() = 123.4, "the return of TestOptDoubleByRefByVal(Optional, Optional) is: " & TestOptDoubleByRefByVal(), "TestOptDouble()"
    TestLog_ASSERT TestOptDoubleByRefByVal(123.4) = 246.8, "the return of TestOptDoubleByRefByVal(Double, Optional) is: " & TestOptDoubleByRefByVal(123.4), "TestOptDouble(123.4)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptDoubleByRefByVal(, 567.8) = 567.8, "the return of TestOptDoubleByRefByVal(Optional, Double) is: " & TestOptDoubleByRefByVal(, 567.8), "TestOptDoubleByRefByVal(, 567.8)"
    TestLog_ASSERT Format(TestOptDoubleByRefByVal(123.4, 567.8), "0.0") = 691.2, "the return of TestOptDoubleByRefByVal(Double, Double) is: " & TestOptDoubleByRefByVal(123.4, 567.8), "TestOptDoubleByRefByVal(123.4, 567.8)"

    ' optionals with integer datatypes
    TestLog_ASSERT TestOptInteger() = 123, "the return of TestOptInteger(Optional, Optional) is: " & TestOptInteger(), "TestOptInteger()"
    TestLog_ASSERT TestOptInteger(123) = 246, "the return of TestOptInteger(Integer, Optional) is: " & TestOptInteger(123), "TestOptInteger(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptInteger(, 456) = 456, "the return of TestOptInteger(Optional, Integer) is: " & TestOptInteger(, 456), "TestOptInteger(, 456)"
    TestLog_ASSERT TestOptInteger(123, 456) = 579, "the return of TestOptInteger(Integer, Integer) is: " & TestOptInteger(123, 456), "TestOptInteger(123, 456)"

    ' optionals with integer datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptIntegerByRefByVal() = 123, "the return of TestOptIntegerByRefByVal(Optional, Optional) is: " & TestOptIntegerByRefByVal(), "TestOptIntegerByRefByVal()"
    TestLog_ASSERT TestOptIntegerByRefByVal(123) = 246, "the return of TestOptIntegerByRefByVal(Integer, Optional) is: " & TestOptIntegerByRefByVal(123), "TestOptIntegerByRefByVal(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptIntegerByRefByVal(, 456) = 456, "the return of TestOptIntegerByRefByVal(Optional, Integer) is: " & TestOptIntegerByRefByVal(, 456), "TestOptIntegerByRefByVal(, 456)"
    TestLog_ASSERT TestOptIntegerByRefByVal(123, 456) = 579, "the return of TestOptIntegerByRefByVal(Integer, Integer) is: " & TestOptIntegerByRefByVal(123, 456), "TestOptIntegerByRefByVal(123, 456)"

    ' optionals with string datatypes
    TestLog_ASSERT TestOptString() = "123", "the return of TestOptString(Optional, Optional) is: " & TestOptString(), "TestOptString()"
    TestLog_ASSERT TestOptString("123") = "123123", "the return of TestOptString(String, Optional) is: " & TestOptString("123"), "TestOptString(""123"")"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptString(, "456") = "456", "the return of TestOptString(Optional, String) is: " & TestOptString(, "456"), "TestOptString(, ""456"")"
    TestLog_ASSERT TestOptString("123", "456") = "123456", "the return of TestOptString(String, String) is: " & TestOptString("123", "456"), "TestOptString(""123"", ""456"")"

    ' optionals with string datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptStringByRefByVal() = "123", "the return of TestOptStringByRefByVal(Optional, Optional) is: " & TestOptStringByRefByVal(), "TestOptStringByRefByVal()"
    TestLog_ASSERT TestOptStringByRefByVal("123") = "123123", "the return of TestOptStringByRefByVal(String, Optional) is: " & TestOptStringByRefByVal("123"), "TestOptStringByRefByVal(""123"")"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptStringByRefByVal(, "456") = "456", "the return of TestOptStringByRefByVal(Optional, String) is: " & TestOptStringByRefByVal(, "456"), "TestOptStringByRefByVal(, ""456"")"
    TestLog_ASSERT TestOptStringByRefByVal("123", "456") = "123456", "the return of TestOptStringByRefByVal(String, String) is: " & TestOptStringByRefByVal("123", "456"), "TestOptStringByRefByVal(""123"", ""456"")"

    ' optionals with object datatypes
    Dim cA As New Collection
    cA.Add (123)
    cA.Add (456)
    Dim cB As New Collection
    cB.Add (123.4)
    cB.Add (567.8)
    TestLog_ASSERT TestOptObject() = IsMissingAB, "the return of TestOptObject(Optional, Optional) is: " & TestOptObject(), "TestOptObject()"
    TestLog_ASSERT TestOptObject(cA) = 579, "the return of TestOptObject(Object, Optional) is: " & TestOptObject(cA), "TestOptObject(A)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT Format(TestOptObject(, cB), "0.0") = 691.2, "the return of TestOptObject(Optional, Object) is: " & TestOptObject(, cB), "TestOptObject(, B)"
    TestLog_ASSERT Format(TestOptObject(cA, cB), "0.0") = 1270.2, "the return of TestOptObject(Object, Object) is: " & TestOptObject(cA, cB), "TestOptObject(A, B)"

    ' optionals with object datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptObjectByRefByVal() = IsMissingAB, "the return of TestOptObjectByRefByVal(Optional, Optional) is: " & TestOptObjectByRefByVal(), "TestOptObjectByRefByVal()"
    TestLog_ASSERT TestOptObjectByRefByVal(cA) = 579, "the return of TestOptObjectByRefByVal(Object, Optional) is: " & TestOptObjectByRefByVal(cA), "TestOptObjectByRefByVal(A)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT Format(TestOptObjectByRefByVal(, cB), "0.0") = 691.2, "the return of TestOptObjectByRefByVal(Optional, Object) is: " & TestOptObjectByRefByVal(, cB), "TestOptObjectByRefByVal(, B)"
    TestLog_ASSERT Format(TestOptObjectByRefByVal(cA, cB), "0.0") = 1270.2, "the return of TestOptObjectByRefByVal(Object, Object) is: " & TestOptObjectByRefByVal(cA, cB), "TestOptObjectByRefByVal(A, B)"

    ' optionals with array datatypes
    Dim aA(0 To 1) As Integer
    aA(0) = 123
    aA(1) = 456
    Dim aB(0 To 1) As Variant
    aB(0) = 123.4
    aB(1) = 567.8
    TestLog_ASSERT TestOptArray() = IsMissingAB, "the return of TestOptArray(Optional, Optional) is: " & TestOptArray(), "TestOptArray()"
    TestLog_ASSERT TestOptArray(aA) = 579, "the return of TestOptArray(Array, Optional) is: " & TestOptArray(aA), "TestOptArray(A)"
    ' TODO - tdf#125180 for more details
    'TestLog_ASSERT Format(TestOptArray(, aB), "0.0") = 691.2, "the return of TestOptArray(Optional, Array) is: " & TestOptArray(, B), "TestOptArray(, B)"
    TestLog_ASSERT Format(TestOptArray(aA, aB), "0.0") = 1270.2, "the return of TestOptArray(Array, Array) is: " & TestOptArray(aA, aB), "TestOptArray(A, B)"

    ' optionals with array datatypes (ByRef and ByVal)
    TestLog_ASSERT TestOptArrayByRefByVal() = IsMissingAB, "the return of TestOptArrayByRefByVal(Optional, Optional) is: " & TestOptArrayByRefByVal(), "TestOptArrayByRefByVal()"
    TestLog_ASSERT TestOptArrayByRefByVal(aA) = 579, "the return of TestOptArrayByRefByVal(Array, Optional) is: " & TestOptArrayByRefByVal(aA), "TestOptArrayByRefByVal(A)"
    ' TODO - tdf#125180 for more details
    'TestLog_ASSERT Format(TestOptArrayByRefByVal(, aB), "0.0") = 691.2, "the return of TestOptArrayByRefByVal(Optional, Array) is: " & TestOptArrayByRefByVal(, B), "TestOptArrayByRefByVal(, B)"
    TestLog_ASSERT Format(TestOptArrayByRefByVal(aA, aB), "0.0") = 1270.2, "the return of TestOptArrayByRefByVal(Array, Array) is: " & TestOptArrayByRefByVal(aA, aB), "TestOptArrayByRefByVal(A, B)"

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testOptionalsBasic = result

    Exit Function
errorHandler:
    TestLog_ASSERT False, testName & ": hit error handler"
End Function

Function TestOptVariant(Optional A, Optional B As Variant = 123)
    If IsMissing(A) And IsMissing(B) Then
        TestOptVariant = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptVariant = B
    ElseIf IsMissing(B) Then
        TestOptVariant = A
    Else
        TestOptVariant = A + B
    End If
End Function

Function TestOptVariantByRefByVal(Optional ByRef A, Optional ByVal B As Variant = 123)
    If IsMissing(A) And IsMissing(B) Then
        TestOptVariantByRefByVal = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptVariantByRefByVal = B
    ElseIf IsMissing(B) Then
        TestOptVariantByRefByVal = A
    Else
        TestOptVariantByRefByVal = A + B
    End If
End Function


Function TestOptDouble(Optional A As Double, Optional B As Double = 123.4)
    If IsMissing(A) And IsMissing(B) Then
        TestOptDouble = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptDouble = B
    ElseIf IsMissing(B) Then
        TestOptDouble = A
    Else
        TestOptDouble = A + B
    End If
End Function

Function TestOptDoubleByRefByVal(Optional ByRef A As Double, Optional ByVal B As Double = 123.4)
    If IsMissing(A) And IsMissing(B) Then
        TestOptDoubleByRefByVal = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptDoubleByRefByVal = B
    ElseIf IsMissing(B) Then
        TestOptDoubleByRefByVal = A
    Else
        TestOptDoubleByRefByVal = A + B
    End If
End Function

Function TestOptInteger(Optional A As Integer, Optional B As Integer = 123)
    If IsMissing(A) And IsMissing(B) Then
        TestOptInteger = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptInteger = B
    ElseIf IsMissing(B) Then
        TestOptInteger = A
    Else
        TestOptInteger = A + B
    End If
End Function

Function TestOptIntegerByRefByVal(Optional ByRef A As Integer, Optional ByVal B As Integer = 123)
    If IsMissing(A) And IsMissing(B) Then
        TestOptIntegerByRefByVal = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptIntegerByRefByVal = B
    ElseIf IsMissing(B) Then
        TestOptIntegerByRefByVal = A
    Else
        TestOptIntegerByRefByVal = A + B
    End If
End Function

Function TestOptString(Optional A As String, Optional B As String = "123")
    If IsMissing(A) And IsMissing(B) Then
        TestOptString = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptString = B
    ElseIf IsMissing(B) Then
        TestOptString = A
    Else
        TestOptString = A & B
    End If
End Function

Function TestOptStringByRefByVal(Optional ByRef A As String, Optional ByVal B As String = "123")
    If IsMissing(A) And IsMissing(B) Then
        TestOptStringByRefByVal = IsMissingAB
    ElseIf IsMissing(A) Then
        TestOptStringByRefByVal = B
    ElseIf IsMissing(B) Then
        TestOptStringByRefByVal = A
    Else
        TestOptStringByRefByVal = A & B
    End If
End Function

Function TestOptObject(Optional A As Collection, Optional B As Collection)
    Dim idx As Integer
    If IsMissing(A) And IsMissing(B) Then
        TestOptObject = IsMissingAB
    ElseIf IsMissing(A) Then
        For idx = 1 To B.Count
            TestOptObject = TestOptObject + B.Item(idx)
        Next idx
    ElseIf IsMissing(B) Then
        For idx = 1 To A.Count
            TestOptObject = TestOptObject + A.Item(idx)
        Next idx
    Else
        For idx = 1 To A.Count
            TestOptObject = TestOptObject + A.Item(idx)
        Next idx
        For idx = 1 To B.Count
            TestOptObject = TestOptObject + B.Item(idx)
        Next idx
    End If
End Function

Function TestOptObjectByRefByVal(Optional ByRef A As Collection, Optional ByVal B As Collection)
    Dim idx As Integer
    If IsMissing(A) And IsMissing(B) Then
        TestOptObjectByRefByVal = IsMissingAB
    ElseIf IsMissing(A) Then
        For idx = 1 To B.Count
            TestOptObjectByRefByVal = TestOptObjectByRefByVal + B.Item(idx)
        Next idx
    ElseIf IsMissing(B) Then
        For idx = 1 To A.Count
            TestOptObjectByRefByVal = TestOptObjectByRefByVal + A.Item(idx)
        Next idx
    Else
        For idx = 1 To A.Count
            TestOptObjectByRefByVal = TestOptObjectByRefByVal + A.Item(idx)
        Next idx
        For idx = 1 To B.Count
            TestOptObjectByRefByVal = TestOptObjectByRefByVal + B.Item(idx)
        Next idx
    End If
End Function

Function TestOptArray(Optional A() As Integer, Optional B() As Variant)
    Dim idx As Integer
    If IsMissing(A) And IsMissing(B) Then
        TestOptArray = IsMissingAB
    ElseIf IsMissing(A) Then
        For idx = LBound(B) To UBound(B)
            TestOptArray = TestOptArray + B(idx)
        Next idx
    ElseIf IsMissing(B) Then
        For idx = LBound(A) To UBound(A)
            TestOptArray = TestOptArray + A(idx)
        Next idx
    Else
        For idx = LBound(A) To UBound(A)
            TestOptArray = TestOptArray + A(idx)
        Next idx
        For idx = LBound(B) To UBound(B)
            TestOptArray = TestOptArray + B(idx)
        Next idx
    End If
End Function

Function TestOptArrayByRefByVal(Optional ByRef A() As Integer, Optional ByVal B() As Variant)
    Dim idx As Integer
    If IsMissing(A) And IsMissing(B) Then
        TestOptArrayByRefByVal = IsMissingAB
    ElseIf IsMissing(A) Then
        For idx = LBound(B) To UBound(B)
            TestOptArrayByRefByVal = TestOptArrayByRefByVal + B(idx)
        Next idx
    ElseIf IsMissing(B) Then
        For idx = LBound(A) To UBound(A)
            TestOptArrayByRefByVal = TestOptArrayByRefByVal + A(idx)
        Next idx
    Else
        For idx = LBound(A) To UBound(A)
            TestOptArrayByRefByVal = TestOptArrayByRefByVal + A(idx)
        Next idx
        For idx = LBound(B) To UBound(B)
            TestOptArrayByRefByVal = TestOptArrayByRefByVal + B(idx)
        Next idx
    End If
End Function

Sub TestLog_ASSERT(assertion As Boolean, Optional testId As String, Optional testComment As String)

    If assertion = True Then
        passCount = passCount + 1
    Else
        Dim testMsg As String
        If Not IsMissing(testId) Then
            testMsg = testMsg + " : " + testId
        End If
        If Not IsMissing(testComment) And Not (testComment = "") Then
            testMsg = testMsg + " (" + testComment + ")"
        End If

        result = result & Chr$(10) & " Failed: " & testMsg
        failCount = failCount + 1
    End If

End Sub