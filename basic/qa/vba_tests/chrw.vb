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
    verify_testCHRW
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCHRW()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(ChrW(87),  "W", "ChrW(87)")
    TestUtil.AssertEqual(ChrW(105), "i", "ChrW(105)")
    TestUtil.AssertEqual(ChrW(35),  "#", "ChrW(35)")

    ' tdf#145693 - argument name should be 'charcode' instead of 'string'
    TestUtil.AssertEqual(ChrW(charcode:=35),  "#", "ChrW(charcode:=35)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCHRW", Err, Error$, Erl)
End Sub
