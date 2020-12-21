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
    verify_testMonth
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testMonth()
    On Error GoTo errorHandler

    Dim ldate As Date
    ldate = 32616
    TestUtil.AssertEqual(Month(ldate),         4, "Month(ldate)")
    TestUtil.AssertEqual(Month("01/02/2007"),  2, "Month(""01/02/2007"")")
    TestUtil.AssertEqual(Month(1),            12, "Month(1)")
    TestUtil.AssertEqual(Month(60),            2, "Month(60)")
    TestUtil.AssertEqual(Month(2000),          6, "Month(2000)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testMonth", Err, Error$, Erl)
End Sub
