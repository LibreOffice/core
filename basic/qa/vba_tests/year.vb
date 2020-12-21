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
    verify_testYear
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testYear()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Year("12/2/1969"), 1969, "Year(""12/2/1969"")")
    TestUtil.AssertEqual(Year(256),         1900, "Year(256)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testYear", Err, Error$, Erl)
End Sub
