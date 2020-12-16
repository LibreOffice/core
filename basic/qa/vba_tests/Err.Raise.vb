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
    TestUtilModule.TestInit()
    '                  test_Description     | Err # | Err_Source     | Err_Description
    Call TestErrRaise("MAXimum error value", MAX_ERR, "doUnitTest.vb", "Custom Error Maximum value")
    Call TestErrRaise("Positive custom error",  1789, ""             , "User-Defined Error Number")
    Call TestErrRaise("Negative custom error", -1793, "doUnitTest.vb", "Negative User-Defined Error Number")
    Call TestErrRaise("MINimum error value", MIN_ERR, ""             , "Custom Error Minimum value")

    TestUtilModule.TestEnd()
    doUnitTest = TestUtilModule.GetResult()
End Function

Sub TestErrRaise(TestName As String, CurErrNo As Long, CurErrSource As String, CurErrDescription As String)
    Dim origPassCount As Integer, origFailCount As Integer

try: On Error Goto catch
    Dim errorHandled As Integer
    Err.Raise(CurErrNo, CurErrSource, CurErrDescription, "", "")

    TestUtilModule.AssertTrue(errorHandled = 1, TestName, "error handler did not execute!")
    TestUtilModule.AssertTrue(Erl = 0, TestName, "Erl = " & Erl)
    TestUtilModule.AssertTrue(Err = 0, TestName, "Err = " & Err)
    TestUtilModule.AssertTrue(Error = "", TestName, "Error = " & Error)
    TestUtilModule.AssertTrue(Err.Description = "", "Err.Description  reset", "Err.Description = "& Err.Description)
    TestUtilModule.AssertTrue(Err.Number = 0, "Err.Number reset", "Err.Number = " & Err.Number)
    TestUtilModule.AssertTrue(Err.Source = "", "Err.Source reset", "Err.Source = " & Err.Source)
    Exit Sub

catch:
    TestUtilModule.AssertTrue(Err.Number = CurErrNo, "Err.Number failure", "Err.Number = " & Err.Number)
    TestUtilModule.AssertTrue(Err.Source = CurErrSource, "Err.Source failure", "Err.Source = " & Err.Source)
    TestUtilModule.AssertTrue(Err.Description = CurErrDescription, "Err.Description failure", "Err.Description = " & Err.Description)

    TestUtilModule.AssertTrue(Erl = 33, "line# failure", "Erl = " & Erl ' WATCH OUT for HARDCODED LINE # HERE !)
    TestUtilModule.AssertTrue(Err = CurErrNo, "Err# failure", "Err = " & Err)
    TestUtilModule.AssertTrue(Error = CurErrDescription, "Error description failure", "Error$ = " & Error$)

    errorHandled = 1
    Resume Next ' Err object properties reset from here …
End Sub
