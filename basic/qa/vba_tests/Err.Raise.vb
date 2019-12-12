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

Function doUnitTest()
    ''' test_vba.cxx main entry point '''
    Call ERR_TestCases
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = result
    Else
        doUnitTest = "OK"
    End If
End Function


Sub ERR_TestCases()
    ''' Raise a User-Defined Errors as Int32 '''
    Const _SOURCE = "ERR.Raise_TestCases"
    passCount = 0
    failCount = 0

    result = "Test Results" & vbNewLine & "============" & vbNewLine

try:
    On Error Goto catch

    Err.Raise(1789, "", "User-Defined Error Number", "", "")
a:  TestLog_ASSERT Err.Number = 1789, _
        "case 10", "Error="& Err.Number
    TestLog_ASSERT Err.Source="", _
        "case 11", "Source: " & Err.Source
    TestLog_ASSERT Err.Source="", _
        "case 12", "Err.Description: " & Err.Description
    Resume _a
_a: On Error goto catch

    Err.Raise(-1793, _SOURCE, "Negative User-Defined Error Number", "", "")
b:  TestLog_ASSERT Err=-1793, _
        "case 20", "Error=" & Err
    TestLog_ASSERT Err.Source=_SOURCE, _
        "case 21", "Source: " & Err.Source
    TestLog_ASSERT Error$ = "Negative User-Defined Error Number", _
        "case 22", "Err.Description: " & Error$

finally:
    result = result & vbNewLine & "Tests passed: " & passCount & vbNewLine & "Tests failed: " & failCount & vbNewLine
    Exit Sub

catch: ' Accept / Reject error number
    Select Case Err.Number
        Case  1789: Goto a:
        Case -1793: Goto b
        Case Else:
            TestLog_ASSERT (False), "ERROR", "#"& Str(Err.Number) &" in '"& _SOURCE &"' at line"& Str(Erl) &" - "& Error$
            Resume Next
    End Select
End Sub

Sub DEV_TEST : doUnitTest : MsgBox result : End Sub

Sub TestLog_ASSERT(assertion As Boolean, Optional testId As String, Optional testComment As String)

    If assertion = True Then
        passCount = passCount + 1
    Else
        Dim testMsg As String
        If Not IsMissing(testId) Then
            testMsg = testMsg + testId + ":"
        End If
        If Not IsMissing(testComment) And Not (testComment = "") Then
            testMsg = testMsg + " (" + testComment + ")"
        End If

        result = result & vbNewLine & "Failed: " & testMsg
        failCount = failCount + 1
    End If

End Sub
