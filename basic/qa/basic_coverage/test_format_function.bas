'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
    TestUtil.TestInit
    verify_testFormat
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testFormat
    On Error GoTo errorHandler

    Dim d As Date
    d = "2024-09-16 17:03:30"
    TestUtil.AssertEqual(Format(d, "YYYY-MM-DD"), "2024-09-16", "Format(d, ""YYYY-MM-DD"")")
    TestUtil.AssertEqual(Format("2024-09-16 05:03:30 PM", "hh-mm-ss"), "17-03-30", "Format(""2024-09-16 05:03:30 PM"", ""hh-mm-ss"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testFormat", Err, Error$, Erl)
End Sub
