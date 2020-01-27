Option VBASupport 1
Option Compatible

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Global Const IsMissingNone = -1
Global Const IsMissingA = 0
Global Const IsMissingAB = 1
Global Const IsMissingAB = 2

Function doUnitTest() As String
    result = verify_testOptionalsVBA()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function

' tdf#36737 - Test optionals with different datatypes. In LO Basic
' with option VBASupport, optional parameters are allowed with default values.
' In addition, having types other than Variant will be initialized to
' their respective default values of its datatype.
Function verify_testOptionalsVBA() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)
    testName = "Test IsMissing (Basic) function"
    On Error GoTo errorHandler

    ' optionals with variant datatypes
    TestLog_ASSERT TestOptVariant() = IsMissingAB, "the return of TestOptVariant(Optional, Optional) is: " & TestOptVariant(), "TestOptVariant()"
    TestLog_ASSERT TestOptVariant(123) = 123, "the return of TestOptVariant(Variant, Optional) is: " & TestOptVariant(123), "TestOptVariant(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptVariant(, 456) = 456, "the return of TestOptVariant(Optional, Variant) is: " & TestOptVariant(, 456), "TestOptVariant(, 456)"
    TestLog_ASSERT TestOptVariant(123, 456) = 579, "the return of TestOptVariant(Variant, Variant) is: " & TestOptVariant(123, 456), "TestOptVariant(123, 456)"

    ' optionals with double datatypes
    TestLog_ASSERT TestOptDouble() = 123.4, "the return of TestOptDouble(Optional, Optional) is: " & TestOptDouble(), "TestOptDouble()"
    TestLog_ASSERT TestOptDouble(123.4) = 246.8, "the return of TestOptDouble(Double, Optional) is: " & TestOptDouble(123.4), "TestOptDouble(123.4)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptDouble(, 567.8) = 567.8, "the return of TestOptDouble(Optional, Double) is: " & TestOptDouble(, 567.8), "TestOptDouble(, 567.8)"
    TestLog_ASSERT Format(TestOptDouble(123.4, 567.8), "0.0") = 691.2, "the return of TestOptDouble(Double, Double) is: " & TestOptDouble(123.4, 567.8), "TestOptDouble(123.4, 567.8)"

    ' optionals with integer datatypes
    TestLog_ASSERT TestOptInteger() = 123, "the return of TestOptInteger(Optional, Optional) is: " & TestOptInteger(), "TestOptInteger()"
    TestLog_ASSERT TestOptInteger(123) = 246, "the return of TestOptInteger(Integer, Optional) is: " & TestOptInteger(123), "TestOptInteger(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptInteger(, 456) = 456, "the return of TestOptInteger(Optional, Integer) is: " & TestOptInteger(, 456), "TestOptInteger(, 456)"
    TestLog_ASSERT TestOptInteger(123, 456) = 579, "the return of TestOptInteger(Integer, Integer) is: " & TestOptInteger(123, 456), "TestOptInteger(123, 456)"

    ' optionals with string datatypes
    TestLog_ASSERT TestOptString() = "123", "the return of TestOptString(Optional, Optional) is: " & TestOptString(), "TestOptString()"
    TestLog_ASSERT TestOptString("123") = "123123", "the return of TestOptString(String, Optional) is: " & TestOptString("123"), "TestOptString(""123"")"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptString(, "456") = "456", "the return of TestOptString(Optional, String) is: " & TestOptString(, "456"), "TestOptString(, ""456"")"
    TestLog_ASSERT TestOptString("123", "456") = "123456", "the return of TestOptString(String, String) is: " & TestOptString("123", "456"), "TestOptString(""123"", ""456"")"

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testOptionalsVBA = result

    Exit Function
errorHandler:
    TestLog_ASSERT False, testName & ": hit error handler"
End Function

Function TestOptVariant(Optional A, Optional B)
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