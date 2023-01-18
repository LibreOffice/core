' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_Right_method
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_Right_method
    On Error GoTo errorHandler

    ' RIGHT
    TestUtil.AssertEqual(Right("abc", 2), "bc", "Right(""abc"",2)")

    ' tdf#141474 keyword names need to match that of VBA
    TestUtil.AssertEqual(Right(Length:=4, String:="sometext"), "text", "Right(Length:=4, String:=""sometext"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_Right_method", Err, Error$, Erl)
End Sub
