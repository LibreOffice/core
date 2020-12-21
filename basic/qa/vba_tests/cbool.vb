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
    verify_testCBool
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCBool()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(CBool(1),       True,  "CBool(1)")
    TestUtil.AssertEqual(CBool(1 = 2),   False, "CBool(1 = 2)")
    TestUtil.AssertEqual(CBool(0),       False, "CBool(0)")
    TestUtil.AssertEqual(CBool(21),      True,  "CBool(21)")
    TestUtil.AssertEqual(CBool("true"),  True,  "CBool(""true"")")
    TestUtil.AssertEqual(CBool("false"), False, "CBool(""false"")")
    TestUtil.AssertEqual(CBool("1"),     True,  "CBool(""1"")")
    TestUtil.AssertEqual(CBool("-1"),    True,  "CBool(""-1"")")
    TestUtil.AssertEqual(CBool("0"),     False, "CBool(""0"")")

    Dim a1, a2 As Integer
    a1 = 1: a2 = 10
    TestUtil.AssertEqual(CBool(a1 = a2), False, "CBool(a1 = a2)")
    a1 = 10: a2 = 10
    TestUtil.AssertEqual(CBool(a1 = a2), True,  "CBool(a1 = a2)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCBool", Err, Error$, Erl)
End Sub
