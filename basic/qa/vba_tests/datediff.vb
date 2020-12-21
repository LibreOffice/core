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
    verify_testDateDiff
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testDateDiff()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(DateDiff("yyyy", "22/11/2003", "22/11/2013"),                             10, "DateDiff(""yyyy"", ""22/11/2003"", ""22/11/2013"")")
    TestUtil.AssertEqual(DateDiff("q", "22/11/2003", "22/11/2013"),                                40, "DateDiff(""q"", ""22/11/2003"", ""22/11/2013"")")
    TestUtil.AssertEqual(DateDiff("m", "22/11/2003", "22/11/2013"),                               120, "DateDiff(""m"", ""22/11/2003"", ""22/11/2013"")")
    TestUtil.AssertEqual(DateDiff("y", "22/11/2003", "22/11/2013"),                              3653, "DateDiff(""y"", ""22/11/2003"", ""22/11/2013"")")
    TestUtil.AssertEqual(DateDiff("d", "22/11/2003", "22/11/2013"),                              3653, "DateDiff(""d"", ""22/11/2003"", ""22/11/2013"")")
    TestUtil.AssertEqual(DateDiff("w", "22/11/2003", "22/11/2013"),                               521, "DateDiff(""w"", ""22/11/2003"", ""22/11/2013"")")
    TestUtil.AssertEqual(DateDiff("ww", "22/11/2003", "22/11/2013"),                              522, "DateDiff(""ww"", ""22/11/2003"", ""22/11/2013"")")
    TestUtil.AssertEqual(DateDiff("h", "22/11/2003", "22/11/2013"),                             87672, "DateDiff(""h"", ""22/11/2003"", ""22/11/2013"")")
    TestUtil.AssertEqual(DateDiff("n", "22/11/2012", "22/11/2013"),                            525600, "DateDiff(""n"", ""22/11/2012"", ""22/11/2013"")")
    TestUtil.AssertEqual(DateDiff("s", "22/10/2013", "22/11/2013"),                           2678400, "DateDiff(""s"", ""22/10/2013"", ""22/11/2013"")")
    TestUtil.AssertEqual(DateDiff("d", "22/11/2003", "22/11/2013", vbFriday),                    3653, "DateDiff(""d"", ""22/11/2003"", ""22/11/2013"", vbFriday)")
    TestUtil.AssertEqual(DateDiff("d", "22/11/2003", "22/11/2013", vbMonday),                    3653, "DateDiff(""d"", ""22/11/2003"", ""22/11/2013"", vbMonday)")
    TestUtil.AssertEqual(DateDiff("d", "22/12/2003", "22/11/2013", vbSaturday),                  3623, "DateDiff(""d"", ""22/12/2003"", ""22/11/2013"", vbSaturday)")
    TestUtil.AssertEqual(DateDiff("d", "22/10/2003", "22/11/2013", vbSunday),                    3684, "DateDiff(""d"", ""22/10/2003"", ""22/11/2013"", vbSunday)")
    TestUtil.AssertEqual(DateDiff("d", "22/11/2003", "22/11/2013", vbThursday),                  3653, "DateDiff(""d"", ""22/11/2003"", ""22/11/2013"", vbThursday)")
    TestUtil.AssertEqual(DateDiff("d", "22/11/2003", "22/11/2013", vbTuesday),                   3653, "DateDiff(""d"", ""22/11/2003"", ""22/11/2013"", vbTuesday)")
    TestUtil.AssertEqual(DateDiff("d", "22/11/2003", "22/11/2013", vbFriday, vbFirstJan1),       3653, "DateDiff(""d"", ""22/11/2003"", ""22/11/2013"", vbFriday, vbFirstJan1)")
    TestUtil.AssertEqual(DateDiff("d", "22/11/2003", "22/11/2013", vbThursday, vbFirstFourDays), 3653, "DateDiff(""d"", ""22/11/2003"", ""22/11/2013"", vbThursday, vbFirstFourDays)")
    TestUtil.AssertEqual(DateDiff("d", "22/11/2003", "22/11/2013", vbSunday, vbFirstFullWeek),   3653, "DateDiff(""d"", ""22/11/2003"", ""22/11/2013"", vbSunday, vbFirstFullWeek)")
    TestUtil.AssertEqual(DateDiff("d", "22/11/2003", "22/11/2013", vbSaturday, vbFirstFullWeek), 3653, "DateDiff(""d"", ""22/11/2003"", ""22/11/2013"", vbSaturday, vbFirstFullWeek)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testDateDiff", Err, Error$, Erl)
End Sub
