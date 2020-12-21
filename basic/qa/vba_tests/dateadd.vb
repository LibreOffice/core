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
    verify_testDateAdd
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testDateAdd()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(DateAdd("m", 1, "1995-01-31"),    CDate("1995-02-28"), "DateAdd(""m"", 1, ""1995-01-31"")")
    TestUtil.AssertEqual(DateAdd("m", 1, "1995-01-31"),    CDate("1995-02-28"), "DateAdd(""m"", 1, ""1995-01-31"")")
    TestUtil.AssertEqual(DateAdd("m", 1, "1995-01-31"),    CDate("1995-02-28"), "DateAdd(""m"", 1, ""1995-01-31"")")
    TestUtil.AssertEqual(DateAdd("yyyy", 1, "1995-01-31"), CDate("1996-01-31"), "DateAdd(""yyyy"", 1, ""1995-01-31"")")
    TestUtil.AssertEqual(DateAdd("q", 1, "1995-01-31"),    CDate("1995-04-30"), "DateAdd(""q"", 1, ""1995-01-31"")")
    TestUtil.AssertEqual(DateAdd("y", 1, "1995-01-31"),    CDate("1995-02-01"), "DateAdd(""y"", 1, ""1995-01-31"")")
    TestUtil.AssertEqual(DateAdd("d", 1, "1995-01-31"),    CDate("1995-02-01"), "DateAdd(""d"", 1, ""1995-01-31"")")
    TestUtil.AssertEqual(DateAdd("w", 1, "1995-01-31"),    CDate("1995-02-01"), "DateAdd(""w"", 1, ""1995-01-31"")")
    TestUtil.AssertEqual(DateAdd("ww", 1, "1995-01-31"),   CDate("1995-02-07"), "DateAdd(""ww"", 1, ""1995-01-31"")")

Rem This fails when directly comparing using AssertEqual, probably due to rounding.
    TestUtil.AssertEqualApprox(DateAdd("h", 1, "1995-01-01 21:48:29"), CDate("1995-01-01 22:48:29"), 1E-10, "DateAdd(""h"", 1, ""1995-01-01 21:48:29"")")

    TestUtil.AssertEqual(DateAdd("n", 1, "1995-01-31 21:48:29"), CDate("1995-01-31 21:49:29"), "DateAdd(""n"", 1, ""1995-01-31 21:48:29"")")
    TestUtil.AssertEqual(DateAdd("s", 1, "1995-01-31 21:48:29"), CDate("1995-01-31 21:48:30"), "DateAdd(""s"", 1, ""1995-01-31 21:48:29"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testDateAdd", Err, Error$, Erl)
End Sub
