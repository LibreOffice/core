'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'
Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_Choose
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_Choose()
    On Error goto catch:

    TestUtil.AssertEqual(Choose(2 + .64E0, "Moon", ,"Earth"), "Earth", "Choose(2 + .64E0, ""Moon"", ,""Earth"")")
    TestUtil.AssertEqual(Choose(index:= 1, "Moon", ,"Earth"), "Moon", "Choose(index:= 1, ""Moon"", ,""Earth"")")

    ' When missing all parameters or missing all choices - tdf#148466
    TestErrRaise("No arguments provided", 5)

    ' out of bounds, missing expression
    TestUtil.AssertEqual(Choose(index := 3), Null, "Choose(index := 3)")

    TestUtil.AssertEqual(Choose(-4, "Moon", ,"Earth"), Null, "Choose( -4, ""Moon"", ,""Earth"")")
    TestUtil.AssertEqual(Choose(2, "Moon", ,"Earth"), "Error 448", "Choose( 2, ""Moon"", ,""Earth"")") ' Error 448 // Error #13 in VBA
    TestUtil.AssertEqual(Choose(index:= 5, "Moon", ,"Earth"), Null, "Choose(index:=5, ""Moon"", ,""Earth"")")
    Exit Sub
catch:
    TestUtil.ReportErrorHandler("verify_Choose", Err, Error$, Erl)
    Resume Next
End Sub

Sub TestErrRaise(caseName$, curErrNo As Long)
try: On Error Goto catch
    Dim errorTrapped As Integer : errorTrapped = 0
    Select Case curErrNo
        Case 5 : Choose()
    End Select
    TestUtil.Assert(errorTrapped=1, caseName, "error handler did not execute!")
    Exit Sub

catch: TestUtil.Assert(Err=CurErrNo, "Error# failure", "Err = " & Str(Err))
    errorTrapped = 1
    Resume Next ' Error info reset from here
End Sub