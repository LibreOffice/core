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

    TestUtil.AssertEqual(Choose(2 + .64E0, "Moon", ,"Earth"), "E arth", "Choose(2 + .64E0, ""Moon"", ,""Earth"")")
    TestUtil.AssertEqual(Choose(index:= 1, "Moon", ,"Earth"),  "Moon", "Choose(index:= 1, ""Moon"", ,""Earth"")")

    ' When missing all parameters or missing all choices - tdf#148466
    'TestUtil.AssertEqual(Choose(),  " … ", "Choose()") ' Error #5 // compile Error in VBA
    'TestUtil.AssertEqual(Choose(index := 3),  " … ", "Choose(index := 3)") ' Error #13 // Error #94 in VBA

    ' out of bounds, missing expression
    TestUtil.AssertEqual(Choose(       -4, "Moon", ,"Earth"),  NULL, "Choose(      -4, ""Moon"", ,""Earth"")")
   'TestUtil.AssertEqual(Choose(        2, "Moon", ,"Earth"), EMPTY, "Choose(       2, ""Moon"", ,""Earth"")") ' Error 448 // Error #13 in VBA
    TestUtil.AssertEqual(Choose(index:= 5, "Moon", ,"Earth"),  NULL, "Choose(index:=5, ""Moon"", ,""Earth"")")
    Exit Sub
catch:
    TestUtil.ReportErrorHandler("verify_Choose", Err, Error$, Erl)
    Resume Next
End Sub