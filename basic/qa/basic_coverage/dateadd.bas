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
    verify_testDateAdd
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testDateAdd()
    On Error GoTo errorHandler

    ' tdf#117612
    TestUtil.AssertEqual(DateAdd("m", 1, "2014-01-29"), CDate("2014-02-28"), "DateAdd(""m"", 1, ""2014-01-29"")")

    ' tdf#114011 Without the fix in place, this test would have failed with
    ' returned 01/31/32767, expected 12/31/32767
    TestUtil.AssertEqual(DateAdd("m", 1, 31012004), CDate("32767-12-31"), "DateAdd(""m"", 1, 31012004)")
    TestUtil.AssertEqual(DateAdd("M", 1, 31012005), CDate("32767-12-31"), "DateAdd(""M"", 1, 31012005)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testDateAdd", Err, Error$, Erl)
End Sub
