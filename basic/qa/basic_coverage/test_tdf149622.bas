'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit On

Function doUnitTest() As String
    TestUtil.TestInit
    verify_tdf149622()
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_tdf149622()
    On Error GoTo errorHandler

    Dim oHandler
    oHandler = CreateUNoListener("Handler_", "com.sun.star.awt.XEventHandler")
    TestUtil.AssertEqual(oHandler.handleEvent(0), True, "oHandler.handleEvent(0)")
    ' Before the fix for tdf#149622, this returned the previous return value
    TestUtil.AssertEqual(oHandler.handleEvent(1), False, "oHandler.handleEvent(1)")
    Exit Sub

errorHandler:
    TestUtil.ReportErrorHandler("verify_tdf149622", Err, Error$, Erl)
End Sub

Function Handler_handleEvent(Event) As Boolean
    If Event = 0 Then Handler_handleEvent = True
    ' Do not define return value explicitly in Else case
End Function
