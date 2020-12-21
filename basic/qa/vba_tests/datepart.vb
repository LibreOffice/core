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
    verify_testDatePart
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testDatePart()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(DatePart("yyyy", "1969-02-12"),     1969, "DatePart(""yyyy"", ""1969-02-12"")")
    TestUtil.AssertEqual(DatePart("q", "1969-02-12"),           1, "DatePart(""q"", ""1969-02-12"")")
    TestUtil.AssertEqual(DatePart("y", "1969-02-12"),          43, "DatePart(""y"", ""1969-02-12"")")
    TestUtil.AssertEqual(DatePart("d", "1969-02-12"),          12, "DatePart(""d"", ""1969-02-12"")")
    TestUtil.AssertEqual(DatePart("w", "1969-02-12"),           4, "DatePart(""w"", ""1969-02-12"")")
    TestUtil.AssertEqual(DatePart("ww", "1969-02-12"),          7, "DatePart(""ww"", ""1969-02-12"")")
    TestUtil.AssertEqual(DatePart("h", "1969-02-12 16:32:00"), 16, "DatePart(""h"", ""1969-02-12 16:32:00"")")
    TestUtil.AssertEqual(DatePart("n", "1969-02-12 16:32:00"), 32, "DatePart(""n"", ""1969-02-12 16:32:00"")")
    TestUtil.AssertEqual(DatePart("s", "1969-02-12 16:32:00"),  0, "DatePart(""s"", ""1969-02-12 16:32:00"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testDatePart", Err, Error$, Erl)
End Sub
