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
    verify_testHex
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testHex()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Hex(9),     "9", "Hex(9)")
    TestUtil.AssertEqual(Hex(10),    "A", "Hex(10)")
    TestUtil.AssertEqual(Hex(16),   "10", "Hex(16)")
    TestUtil.AssertEqual(Hex(255),  "FF", "Hex(255)")
    TestUtil.AssertEqual(Hex(256), "100", "Hex(256)")
    TestUtil.AssertEqual(Hex(459), "1CB", "Hex(459)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testHex", Err, Error$, Erl)
End Sub
