' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_Mid_method
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_Mid_method
    On Error GoTo errorHandler

    ' tdf#141474 keyword names need to match that of VBA
    TestUtil.AssertEqual(Mid(start:=6, string:="LibreOffice" ),            "Office", "Mid() with 2 keyword names" )
    TestUtil.AssertEqual(Mid(length:=5, start:=1, string:="LibreOffice" ), "Libre",  "Mid() with 3 keyword names" )

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_Mid_method", Err, Error$, Erl)
End Sub