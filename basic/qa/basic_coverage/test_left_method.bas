' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_Left_method
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_Left_method
    On Error GoTo errorHandler

    ' LEFT
    TestUtil.AssertEqual(Left("abc", 2), "ab", "Left(""abc"",2)")

    ' tdf#141474 keyword names need to match that of VBA
    TestUtil.AssertEqual(Left(Length:=4, String:="sometext"), "some", "Left(Length:=4, String:=""sometext"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_Left_method", Err, Error$, Erl)
End Sub
