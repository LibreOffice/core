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
    verify_testCdbl
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCdbl()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(CDbl(0),                 0,               "CDbl(0)")
    TestUtil.AssertEqual(CDbl(10.1234567890123), 10.1234567890123, "CDbl(10.1234567890123)")
    TestUtil.AssertEqual(CDbl(0.005 * 0.01),      0.00005,         "CDbl(0.005 * 0.01)")

    ' tdf#146672 - skip spaces and tabs at the end of the scanned string
    TestUtil.AssertEqual(CDbl("28.8 "), 28.8, "CDbl(""28.8 "")")
    TestUtil.AssertEqual(CDbl("28.8	"), 28.8, "CDbl(""28.8	"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCdbl", Err, Error$, Erl)
End Sub
