'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Option Explicit

Function doUnitTest()
    ''' This routine is QA/…/test_vba.cxx main entry point '''
    Const MIN_ERR = &hFFFFFFFF : Const MAX_ERR = 2^31-1

    ''' Raise one-to-many User-Defined Errors as signed Int32 '''
    TestUtil.TestInit()
    '                  test_Description     | Err # | Err_Source     | Err_Description
    Call TestErrRaise("MAXimum error value", MAX_ERR, "doUnitTest.vb", "Custom Error Maximum value")
    Call TestErrRaise("Positive custom error",  1789, ""             , "User-Defined Error Number")
    Call TestErrRaise("Negative custom error", -1793, "doUnitTest.vb", "Negative User-Defined Error Number")
    Call TestErrRaise("MINimum error value", MIN_ERR, ""             , "Custom Error Minimum value")

    doUnitTest = TestUtil.GetResult()
End Function

Sub TestErrRaise(TestName As String, CurErrNo As Long, CurErrSource As String, CurErrDescription As String)
    Dim origPassCount As Integer, origFailCount As Integer

try: On Error Goto catch
    Dim errorHandled As Integer
    Err.Raise(CurErrNo, CurErrSource, CurErrDescription, "", "")

    TestUtil.Assert(errorHandled = 1, TestName, "error handler did not execute!")
    TestUtil.Assert(Erl = 0, TestName, "Erl = " & Erl)
    TestUtil.Assert(Err = 0, TestName, "Err = " & Err)
    TestUtil.Assert(Error = "", TestName, "Error = " & Error)
    TestUtil.Assert(Err.Description = "", "Err.Description  reset", "Err.Description = "& Err.Description)
    TestUtil.Assert(Err.Number = 0, "Err.Number reset", "Err.Number = " & Err.Number)
    TestUtil.Assert(Err.Source = "", "Err.Source reset", "Err.Source = " & Err.Source)
    Exit Sub

catch:
    TestUtil.Assert(Err.Number = CurErrNo, "Err.Number failure", "Err.Number = " & Err.Number)
    TestUtil.Assert(Err.Source = CurErrSource, "Err.Source failure", "Err.Source = " & Err.Source)
    TestUtil.Assert(Err.Description = CurErrDescription, "Err.Description failure", "Err.Description = " & Err.Description)

    TestUtil.Assert(Erl = 32, "line# failure", "Erl = " & Erl ' WATCH OUT for HARDCODED LINE # HERE !)
    TestUtil.Assert(Err = CurErrNo, "Err# failure", "Err = " & Err)
    TestUtil.Assert(Error = CurErrDescription, "Error description failure", "Error$ = " & Error$)

    errorHandled = 1
    Resume Next ' Err object properties reset from here …
End Sub
