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
    verify_testDateSerial
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testDateSerial()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(DateSerial(1999, 6, 15),     36326, "DateSerial(1999, 6, 15)")
    TestUtil.AssertEqual(DateSerial(2000, 1 - 7, 15), 36326, "DateSerial(2000, 1 - 7, 15)")
    TestUtil.AssertEqual(DateSerial(1999, 1, 166),    36326, "DateSerial(1999, 1, 166)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testDateSerial", Err, Error$, Erl)
End Sub
