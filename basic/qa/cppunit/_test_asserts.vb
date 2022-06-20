'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Option Explicit

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function GetResult()
    If passCount <> 0 and failCount = 0 Then
        GetResult = "OK"
    Else
        GetResult = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    End If
End Function

Sub TestInit()
    passCount = 0
    failCount = 0
    result = result & "Test Results" & Chr$(10) & "============" & Chr$(10)
End Sub

Sub Assert(Assertion As Boolean, Optional testId As String, Optional testComment As String)
    If Assertion = True Then
        passCount = passCount + 1
    Else
        Dim testMsg As String
        If Not IsMissing(testId) Then
            testMsg = " " + testId
        End If
        If Not IsMissing(testComment) Then
            If Not (testComment = "") Then testMsg = testMsg + " (" + testComment + ")"
        End If

        result = result & Chr$(10) & " Failed:" & testMsg
        failCount = failCount + 1
    End If
End Sub

Sub AssertEqual(actual As Variant, expected As Variant, testName As String)
    If expected = actual Then
        passCount = passCount + 1
    Else
        result = result & Chr$(10) & " Failed: " & testName & " returned " & actual & ", expected " & expected
        failCount = failCount + 1
    End If
End Sub

' Same as AssertEqual, but also checks actual and expected types
Sub AssertEqualStrict(actual As Variant, expected As Variant, testName As String)
    AssertEqual actual, expected, testName
    AssertEqual TypeName(actual), TypeName(expected), testName & " type mismatch:"
End Sub

Sub AssertEqualApprox(actual, expected, epsilon, testName As String)
    If Abs(expected - actual) <= epsilon Then
        passCount = passCount + 1
    Else
        result = result & Chr$(10) & " Failed: " & testName & " returned " & actual & ", expected " & expected & ", epsilon " & epsilon
        failCount = failCount + 1
    End If
End Sub

Sub ReportErrorHandler(testName As String, aErr, sError, nErl)
    Assert False, testName, "hit error handler - " & aErr & ": " & sError & " line : " & nErl
End Sub
