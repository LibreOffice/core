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
    verify_testCDate
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCDate()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(CDate("12/02/1969"), 25246, "CDate(""12/02/1969"")")
    TestUtil.AssertEqual(CDate("07/07/1977"), 28313, "CDate(""07/07/1977"")")
    TestUtil.AssertEqual(CDate(#7/7/1977#),   28313, "CDate(#7/7/1977#)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCDate", Err, Error$, Erl)
End Sub
