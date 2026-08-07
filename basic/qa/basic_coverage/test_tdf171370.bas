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
    verify_tdf171370
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_tdf171370()
    On Error GoTo errorHandler

    Dim aEvent As New com.sun.star.awt.KeyEvent ' KeyChar is a UNO char
    aEvent.KeyChar = 87

    TestUtil.AssertEqual(TypeName(aEvent.KeyChar), "Char", "TypeName(aEvent.KeyChar)")
    TestUtil.AssertEqual(CStr(aEvent.KeyChar), "W", "CStr(aEvent.KeyChar)")
    TestUtil.AssertEqual("" & aEvent.KeyChar, "W", "concatenated aEvent.KeyChar")
    TestUtil.AssertEqual(Len(aEvent.KeyChar), 1, "Len(aEvent.KeyChar)")
    TestUtil.AssertEqual(Asc(aEvent.KeyChar), 87, "Asc(aEvent.KeyChar)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_tdf171370", Err, Error$, Erl)
End Sub
