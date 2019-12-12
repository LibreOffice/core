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
    ''' This routine is QA/…/test_vba.cxx main entry point '''
    passCount = 0 : failCount = 0
    Const MIN_ERR = &hFFFFFFFF : Const MAX_ERR = 2^31-1

    ''' Raise one-to-many User-Defined Errors as signed Int32 '''
    result = "Test Results" & vbNewLine & "============" & vbNewLine
    '                  test_Description     | Err # | Err_Source     | Err_Description
    Call TestErrRaise("MAXimum error value", MAX_ERR, "doUnitTest.vb", "Custom Error Maximum value")
    Call TestErrRaise("Positive custom error",  1789, ""             , "User-Defined Error Number")
    Call TestErrRaise("Negative custom error", -1793, "doUnitTest.vb", "Negative User-Defined Error Number")
    Call TestErrRaise("MINimum error value", MIN_ERR, ""             , "Custom Error Minimum value")

    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = result
    Else
        doUnitTest = "OK"
    End If
End Function

Sub TestErrRaise(TestName As String, CurErrNo As Long, CurErrSource As String, CurErrDescription As String)
    result = result & vbNewLine & TestName
    Dim origPassCount As Integer, origFailCount As Integer
    origPassCount = passCount
    origFailCount = failCount

try: On Error Goto catch
    Err.Raise(CurErrNo, CurErrSource, CurErrDescription, "", "")

    'result = result & vbNewLine & "Testing after error handler"
    TestLog_ASSERT (passCount + failCount) > (origPassCount + origFailCount), TestName, "error handler did not execute!"
    TestLog_ASSERT Erl = 0, TestName, "Erl = " & Erl
    TestLog_ASSERT Err = 0, TestName, "Err = " & Err
    TestLog_ASSERT Error = "", TestName, "Error = " & Error
    TestLog_ASSERT Err.Description = "", "Err.Description  reset", "Err.Description = "& Err.Description
    TestLog_ASSERT Err.Number = 0, "Err.Number reset", "Err.Number = " & Err.Number
    TestLog_ASSERT Err.Source = "", "Err.Source reset", "Err.Source = " & Err.Source
    Exit Sub

catch:
    'result = result & vbNewLine & "Testing in error handler"
    TestLog_ASSERT Err.Number = CurErrNo, "Err.Number failure", "Err.Number = " & Err.Number
    TestLog_ASSERT Err.Source = CurErrSource, "Err.Source failure", "Err.Source = " & Err.Source
    TestLog_ASSERT Err.Description = CurErrDescription, "Err.Description failure", "Err.Description = " & Err.Description

    TestLog_ASSERT Erl = 42, "line# failure", "Erl = " & Erl ' WATCH OUT for HARDCODED LINE # HERE !
    TestLog_ASSERT Err = CurErrNo, "Err# failure", "Err = " & Err
    TestLog_ASSERT Error = CurErrDescription, "Error description failure", "Error$ = " & Error$

    Resume Next ' Err object properties reset from here …
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
