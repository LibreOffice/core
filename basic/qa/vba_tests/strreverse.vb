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
    verify_testStrReverse
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testStrReverse()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(StrReverse("abcd"),   "dcba",    "StrReverse(""abcd"")")
    TestUtil.AssertEqual(StrReverse("ABABAB"), "BABABA",  "StrReverse(""ABABAB"")")
    TestUtil.AssertEqual(StrReverse("123456"), "654321",  "StrReverse(""123456"")")
    TestUtil.AssertEqual(StrReverse(6),        "6",       "StrReverse(6)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testStrReverse", Err, Error$, Erl)
End Sub
