'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testDateAdd
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testDateAdd()
    On Error GoTo errorHandler

    ' tdf#117612
    TestUtil.AssertEqual(DateAdd("m", 1, "2014-01-29"), CDate("2014-02-28"), "DateAdd(""m"", 1, ""2014-01-29"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testDateAdd", Err, Error$, Erl)
End Sub
