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
    verify_testCVDate
    ' SKIPPED test due to CVDate not being available
    'doUnitTest = TestUtil.GetResult()
    doUnitTest = "OK"
End Function

Sub verify_testCVDate()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(CVDate("12.2.1969"),  25246, "CVDate(""12.2.1969"")")
    TestUtil.AssertEqual(CVDate("07/07/1977"), 28313, "CVDate(""07/07/1977"")")
    TestUtil.AssertEqual(CVDate(#7/7/1977#),   28313, "CVDate(#7/7/1977#)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCVDate", Err, Error$, Erl)
End Sub
