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

    'Predefined_Datetime_Format_Sample
    Predefined_Number_Format_Sample
    'Custom_Datetime_Format_Sample
    Custom_Number_Format_Sample
    Custom_Text_Format_Sample
    testFormat

    doUnitTest = TestUtil.GetResult()
End Function

Sub Predefined_Datetime_Format_Sample()
    Dim TestStr As String
    const myDate = "01/06/98"
    const MyTime = "17:08:06"

    On Error GoTo errorHandler

    ' These tests only apply to en_US locale

    ' The date/time format have a little different between ms office and OOo due to different locale and system...
    TestStr = Format(myDate, "General Date") ' 1/6/98
    TestUtil.Assert(IsDate(TestStr), "IsDate(TestStr)", "General Date")
    'TestUtil.AssertEqual(TestStr, "1/6/98", "General Date")

    TestStr = Format(myDate, "Long Date") ' Tuesday, January 06, 1998
    TestUtil.AssertEqual(TestStr, "Tuesday, January 06, 1998", "Long Date")
    'TestUtil.Assert(IsDate(TestStr), "IsDate(TestStr)", "Long Date")

    TestStr = Format(myDate, "Medium Date") ' 06-Jan-98
    'TestUtil.AssertEqual(TestStr, "06-Jan-98", "Medium Date")
    TestUtil.Assert(IsDate(TestStr), "IsDate(TestStr)", "Medium Date")

    TestStr = Format(myDate, "Short Date") ' 1/6/98
    'TestUtil.AssertEqual(TestStr, "1/6/98", "Short Date")
    TestUtil.Assert(IsDate(TestStr), "IsDate(TestStr)", "Short Date")

    TestStr = Format(MyTime, "Long Time") ' 5:08:06 PM
    'TestUtil.AssertEqual(TestStr, "5:08:06 PM", "Long Time")
    TestUtil.Assert(IsDate(TestStr), "IsDate(TestStr)", "Long Time")

    TestStr = Format(MyTime, "Medium Time") ' 05:08 PM
    'TestUtil.AssertEqual(TestStr, "05:08 PM", "Medium Time")
    TestUtil.Assert(IsDate(TestStr), "IsDate(TestStr)", "Medium Time")

    TestStr = Format(MyTime, "Short Time") ' 17:08
    'TestUtil.AssertEqual(TestStr, "17:08", "Short Time")
    TestUtil.Assert(IsDate(TestStr), "IsDate(TestStr)", "Short Time")
    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("Predefined_Datetime_Format_Sample", Err, Error$, Erl)
End Sub

Sub Predefined_Number_Format_Sample()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Format(562486.2356, "General Number"), "562486.2356", "Format(562486.2356, ""General Number"")")
    TestUtil.AssertEqual(Format(0.2, "Fixed"),                  "0.20",        "Format(0.2, ""Fixed"")")
    TestUtil.AssertEqual(Format(562486.2356, "Standard"),       "562,486.24",  "Format(562486.2356, ""Standard"")")
    TestUtil.AssertEqual(Format(0.7521, "Percent"),             "75.21%",      "Format(0.7521, ""Percent"")")
    TestUtil.AssertEqual(Format(562486.2356, "Scientific"),     "5.62E+05",    "Format(562486.2356, ""Scientific"")")
    TestUtil.AssertEqual(Format(-3456.789, "Scientific"),       "-3.46E+03",   "Format(-3456.789, ""Scientific"")")
    TestUtil.AssertEqual(Format(0, "Yes/No"),                   "No",          "Format(0, ""Yes/No"")")
    TestUtil.AssertEqual(Format(23, "Yes/No"),                  "Yes",         "Format(23, ""Yes/No"")")
    TestUtil.AssertEqual(Format(0, "True/False"),               "False",       "Format(0, ""True/False"")")
    TestUtil.AssertEqual(Format(23, "True/False"),              "True",        "Format(23, ""True/False"")")
    TestUtil.AssertEqual(Format(0, "On/Off"),                   "Off",         "Format(0, ""On/Off"")")
    TestUtil.AssertEqual(Format(23, "On/Off"),                  "On",          "Format(23, ""On/Off"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("Predefined_Number_Format_Sample", Err, Error$, Erl)
End Sub

Sub Custom_Datetime_Format_Sample()
    const myDate = "01/06/98"
    const MyTime = "05:08:06"
    const MyTimePM = "17:08:06"

    On Error GoTo errorHandler

    ' These tests only apply to en_US locale
    TestUtil.AssertEqual(Format("01/06/98 17:08:06", "c"),   "1/6/98 5:08:06 PM",         "Format(""01/06/98 17:08:06"", ""c"")")
    TestUtil.AssertEqual(Format(myDate, "dddddd"),           "Tuesday, January 06, 1998", "Format(myDate, ""dddddd"")")
    TestUtil.AssertEqual(Format(myDate, "mm-dd-yyyy"),       "01-06-1998",                "Format(myDate, ""mm-dd-yyyy"")")
    TestUtil.AssertEqual(Format(myDate, "d"),                "6",                         "Format(myDate, ""d"")")
    TestUtil.AssertEqual(Format(myDate, "dd"),               "06",                        "Format(myDate, ""dd"")")
    TestUtil.AssertEqual(Format(myDate, "ddd"),              "Tue",                       "Format(myDate, ""ddd"")")
    TestUtil.AssertEqual(Format(myDate, "dddd"),             "Tuesday",                   "Format(myDate, ""dddd"")")
    TestUtil.AssertEqual(Format(MyTime, "h"),                "5",                         "Format(MyTime, ""h"")")
    TestUtil.AssertEqual(Format(MyTime, "hh"),               "05",                        "Format(MyTime, ""hh"")")
    TestUtil.AssertEqual(Format(MyTime, "n"),                "8",                         "Format(MyTime, ""n"")")
    TestUtil.AssertEqual(Format(MyTime, "nn"),               "08",                        "Format(MyTime, ""nn"")")
    TestUtil.AssertEqual(Format(myDate, "m"),                "1",                         "Format(myDate, ""m"")")
    TestUtil.AssertEqual(Format(myDate, "mm"),               "01",                        "Format(myDate, ""mm"")")
    TestUtil.AssertEqual(Format(myDate, "mmm"),              "Jan",                       "Format(myDate, ""mmm"")")
    TestUtil.AssertEqual(Format(myDate, "mmmm"),             "January",                   "Format(myDate, ""mmmm"")")
    TestUtil.AssertEqual(Format(MyTime, "s"),                "6",                         "Format(MyTime, ""s"")")
    TestUtil.AssertEqual(Format(MyTime, "ss"),               "06",                        "Format(MyTime, ""ss"")")
    TestUtil.AssertEqual(Format(MyTimePM, "hh:mm:ss AM/PM"), "05:08:06 PM",               "Format(MyTimePM, ""hh:mm:ss AM/PM"")")
    TestUtil.AssertEqual(Format(MyTimePM, "hh:mm:ss"),       "17:08:06",                  "Format(MyTimePM, ""hh:mm:ss"")")
    TestUtil.AssertEqual(Format(myDate, "ww"),               "2",                         "Format(myDate, ""ww"")")
    TestUtil.AssertEqual(Format(myDate, "w"),                "3",                         "Format(myDate, ""w"")")
    TestUtil.AssertEqual(Format(myDate, "y"),                "6",                         "Format(myDate, ""y"")")
    TestUtil.AssertEqual(Format(myDate, "yy"),               "98",                        "Format(myDate, ""yy"")")
    TestUtil.AssertEqual(Format(myDate, "yyyy"),             "1998",                      "Format(myDate, ""yyyy"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("Custom_Datetime_Format_Sample", Err, Error$, Erl)
End Sub

Sub Custom_Number_Format_Sample()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Format(23.675, "00.0000"),          "23.6750",      "Format(23.675, ""00.0000"")")
    TestUtil.AssertEqual(Format(23.675, "00.00"),            "23.68",        "Format(23.675, ""00.00"")")
    TestUtil.AssertEqual(Format(2658, "00000"),              "02658",        "Format(2658, ""00000"")")
    TestUtil.AssertEqual(Format(2658, "00.00"),              "2658.00",      "Format(2658, ""00.00"")")
    TestUtil.AssertEqual(Format(23.675, "##.####"),          "23.675",       "Format(23.675, ""##.####"")")
    TestUtil.AssertEqual(Format(23.675, "##.##"),            "23.68",        "Format(23.675, ""##.##"")")
    TestUtil.AssertEqual(Format(12345.25, "#,###.##"),       "12,345.25",    "Format(12345.25, ""#,###.##"")")
    TestUtil.AssertEqual(Format(0.25, "##.00%"),             "25.00%",       "Format(0.25, ""##.00%"")")
    TestUtil.AssertEqual(Format(1000000, "#,###"),           "1,000,000",    "Format(1000000, ""#,###"")")
    TestUtil.AssertEqual(Format(1.09837555, "#.#####E+###"), "1.09838E+000", "Format(1.09837555, ""#.#####E+###"")")
    TestUtil.AssertEqual(Format(1.09837555, "###.####E#"),   "1.0984E0",     "Format(1.09837555, ""###.####E#"")")
    TestUtil.AssertEqual(Format(1098.37555, "###.####E#"),   "1.0984E3",     "Format(1098.37555, ""###.####E#"")")
    TestUtil.AssertEqual(Format(1098375.55, "###.####E#"),   "1.0984E6",     "Format(1098375.55, ""###.####E#"")")
    TestUtil.AssertEqual(Format(1.09837555, "######E#"),     "1E0",          "Format(1.09837555, ""######E#"")")
    TestUtil.AssertEqual(Format(123456.789, "###E0"),        "123E3",        "Format(123456.789, ""###E0"")")
    TestUtil.AssertEqual(Format(123567.89, "###E0"),         "124E3",        "Format(123567.89, ""###E0"")")
    TestUtil.AssertEqual(Format(12, "###E0"),                "12E0",         "Format(12, ""###E0"")")
    TestUtil.AssertEqual(Format(12, "000E0"),                "012E0",        "Format(12, ""000E0"")")
    TestUtil.AssertEqual(Format(0.12345, "###E0"),           "123E-3",       "Format(0.12345, ""###E0"")")
    TestUtil.AssertEqual(Format(123456, "####E0"),           "12E4",         "Format(123456, ""####E0"")")
    TestUtil.AssertEqual(Format(2345.25, "$#,###.##"),       "$2,345.25",    "Format(2345.25, ""$#,###.##"")")
    TestUtil.AssertEqual(Format(0.25, "##.###\%"),           ".25%",         "Format(0.25, ""##.###\%"")")
    TestUtil.AssertEqual(Format(12.25, "0.???"),             "12.25 ",       "Format(12.25, ""0.???"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("Custom_Number_Format_Sample", Err, Error$, Erl)
End Sub

Sub Custom_Text_Format_Sample()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Format("VBA", "<"), "vba", "Format(""VBA"", ""<"")")
    TestUtil.AssertEqual(Format("vba", ">"), "VBA", "Format(""vba"", "">"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("Custom_Text_Format_Sample", Err, Error$, Erl)
End Sub

Sub testFormat()
    On Error GoTo errorHandler

    const TestDateTime = #2001-1-27T17:04:23#
    TestUtil.AssertEqual(Format(TestDateTime, "h:m:s"),            "17:4:23",               "Format(TestDateTime, ""h:m:s"")")
    TestUtil.AssertEqual(Format(TestDateTime, "ttttt"),            "5:04:23 PM",            "Format(TestDateTime, ""ttttt"")")
    TestUtil.AssertEqual(Format(TestDateTime, "dddd, MMM d yyyy"), "Saturday, Jan 27 2001", "Format(TestDateTime, ""dddd, MMM d yyyy"")")
    TestUtil.AssertEqual(Format(TestDateTime, "HH:mm:ss"),         "17:04:23",              "Format(TestDateTime, ""HH:mm:ss"")")

    TestUtil.AssertEqual(Format(23),                               "23",                    "Format(23)")
    TestUtil.AssertEqual(Format(5459.4, "##,##0.00"),              "5,459.40",              "Format(5459.4, ""##,##0.00"")")
    TestUtil.AssertEqual(Format(334.9, "###0.00"),                 "334.90",                "Format(334.9, ""###0.00"")")
    TestUtil.AssertEqual(Format(5, "0.00%"),                       "500.00%",               "Format(5, ""0.00%"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("testFormat", Err, Error$, Erl)
End Sub
