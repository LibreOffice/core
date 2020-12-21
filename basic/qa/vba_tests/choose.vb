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
    verify_testChoose
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testChoose()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Choose(1, "Libre", "Office", "Suite"), "Libre",  "Choose(1, ""Libre"", ""Office"", ""Suite"")")
    TestUtil.AssertEqual(Choose(2, "Libre", "Office", "Suite"), "Office", "Choose(2, ""Libre"", ""Office"", ""Suite"")")
    TestUtil.AssertEqual(Choose(3, "Libre", "Office", "Suite"), "Suite",  "Choose(3, ""Libre"", ""Office"", ""Suite"")")
    TestUtil.Assert(IsNull(Choose(4, "Libre", "Office", "Suite")),  "IsNull(Choose(4, ""Libre"", ""Office"", ""Suite""))")
    TestUtil.Assert(IsNull(Choose(0, "Libre", "Office", "Suite")),  "IsNull(Choose(0, ""Libre"", ""Office"", ""Suite""))")
    TestUtil.Assert(IsNull(Choose(-1, "Libre", "Office", "Suite")), "IsNull(Choose(-1, ""Libre"", ""Office"", ""Suite""))")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testChoose", Err, Error$, Erl)
End Sub
