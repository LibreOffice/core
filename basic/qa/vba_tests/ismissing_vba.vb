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
    result = verify_testIsMissingVBA()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = result
    Else
        doUnitTest = "OK"
    End If
End Function

' tdf#36737 - Test isMissing function with different datatypes. In LO Basic 
' with option VBASupport, optional parameters are allowed with default values. 
' In addition, missing parameters will be initialized to their respective
' default values of its datatype.
Function verify_testIsMissingVBA() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)
    testName = "Test IsMissing (Basic) function"
    On Error GoTo errorHandler

    ' isMissing with variant datatypes
    TestLog_ASSERT TestOptVariant() = IsMissingAB, "the return of IsMissing in TestOptVariant(Optional, Optional) is: " & TestOptVariant(), "TestOptVariant()"
    TestLog_ASSERT TestOptVariant(123) = IsMissingB, "the return of IsMissing in TestOptVariant(Variant, Optional) is: " & TestOptVariant(123), "TestOptVariant(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptVariant(, 123) = IsMissingA, "the return of IsMissing in TestOptVariant(Optional, Variant) is: " & TestOptVariant(, 123), "TestOptVariant(, 123)"
    TestLog_ASSERT TestOptVariant(123, 456) = IsMissingNone, "the return of IsMissing in TestOptVariant(Variant, Variant) is: " & TestOptVariant(123, 456), "TestOptVariant(123, 456)"
    
    ' isMissing with double datatypes
    TestLog_ASSERT TestOptDouble() = IsMissingNone, "the return of IsMissing in TestOptDouble(Optional, Optional with Default value) is: " & TestOptDouble(), "TestOptDouble()"
    TestLog_ASSERT TestOptDouble(123.4) = IsMissingNone, "the return of IsMissing in TestOptDouble(Double, Optional with Default value) is: " & TestOptDouble(123.4), "TestOptDouble(123.4)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptDouble(, 123.4) = IsMissingNone, "the return of IsMissing in TestOptDouble(Optional, Double) is: " & TestOptDouble(, 123.4), "TestOptDouble(, 123.4)"
    TestLog_ASSERT TestOptDouble(123.4, 567.8) = IsMissingNone, "the return of IsMissing in TestOptDouble(Double, Double) is: " & TestOptDouble(123.4, 567.8), "TestOptDouble(123.4, 567.8)"
    
    ' isMissing with integer datatypes
    TestLog_ASSERT TestOptInteger() = IsMissingNone, "the return of IsMissing in TestOptInteger(Optional, Optional with Default value) is: " & TestOptInteger(), "TestOptInteger()"
    TestLog_ASSERT TestOptInteger(123) = IsMissingNone, "the return of IsMissing in TestOptInteger(Integer, Optional with Default value) is: " & TestOptInteger(123), "TestOptInteger(123)"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptInteger(, 123) = IsMissingNone, "the return of IsMissing in TestOptInteger(Optional, Integer) is: " & TestOptInteger(, 123), "TestOptInteger(, 123)"
    TestLog_ASSERT TestOptInteger(123, 456) = IsMissingNone, "the return of IsMissing in TestOptInteger(Integer, Integer) is: " & TestOptInteger(123, 456), "TestOptInteger(123, 456)"
    
    ' isMissing with string datatypes
    TestLog_ASSERT TestOptString() = IsMissingNone, "the return of IsMissing in TestOptString(Optional, Optional with Default value) is: " & TestOptString(), "TestOptString()"
    TestLog_ASSERT TestOptString("123") = IsMissingNone, "the return of IsMissing in TestOptString(String, Optional with Default value) is: " & TestOptString("123"), "TestOptString(""123"")"
    ' TODO - tdf#125180 for more details
    ' TestLog_ASSERT TestOptString(, "123") = IsMissingNone, "the return of IsMissing in TestOptString(Optional, String) is: " & TestOptString(, "123"), "TestOptString(, ""123"")"
    TestLog_ASSERT TestOptString("123", "456") = IsMissingNone, "the return of IsMissing in TestOptString(String, String) is: " & TestOptString("123", "456"), "TestOptString(""123"", ""456"")"
    
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testIsMissingVBA = result

    Exit Function
errorHandler:
    TestLog_ASSERT False, testName & ": hit error handler"
End Function

Function TestOptVariant(Optional A, Optional B)
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