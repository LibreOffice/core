'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testSwitch
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testSwitch()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(MatchUp("Paris"), "French", "MatchUp(""Paris"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testSwitch", Err, Error$, Erl)
End Sub

Function MatchUp(CityName As String)
    MatchUp = Switch(CityName = "London", "English", _
                     CityName = "Rome", "Italian", _
                     CityName = "Paris", "French")
End Function
