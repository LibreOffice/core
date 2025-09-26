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
    ' A string that can be converted to number, with a text-format string
    TestUtil.AssertEqual(Format("001", "foo @ bar"), "foo 001 bar", "Format(""001"", ""foo @ bar"")")
    ' A string that cannot be converted to number, with a text-format string
    TestUtil.AssertEqual(Format("baz", "foo @ bar"), "foo baz bar", "Format(""baz"", ""foo @ bar"")")
    ' Legacy format strings
    ' leading '!': get only the first character of the source string
    TestUtil.AssertEqual(Format("abc", "! @"), "a", "Format(""abc"", ""! @"")")
    ' leading '\': get as many characters from source string, as in format string until the next '\', padding with spaces as needed
    TestUtil.AssertEqual(Format("abcdefgh", "\123\45"), "abcde", "Format(""abcdefgh"", ""\123\45"")")
    TestUtil.AssertEqual(Format("abcdefgh", "\12345"), "abcdef", "Format(""abcdefgh"", ""\12345"")")
    TestUtil.AssertEqual(Format("abc", "\12345\"), "abc    ", "Format(""abc"", ""\12345\"")")
    ' leading '&': get the whole source string unmodified
    TestUtil.AssertEqual(Format("abc", "& @"), "abc", "Format(""abc"", ""& @"")")
    ' non-leading positions
    TestUtil.AssertEqual(Format("abc", "@ !"), "abc !", "Format(""abc"", ""@ !"")")
    TestUtil.AssertEqual(Format("abc", "1\2345"), "abc", "Format(""abc"", ""1\2345"")")
    TestUtil.AssertEqual(Format("abc", "@ &"), "abc &", "Format(""abc"", ""@ &"")")

    TestUtil.AssertEqual(Format(""), "", "Format("""")")
    TestUtil.AssertEqual(Format(" "), " ", "Format("" "")")
    TestUtil.AssertEqual(Format(" 00 "), "0", "Format("" 00 "")")
    TestUtil.AssertEqual(Format(CDate("2025-09-26")), "09/26/2025", "Format(CDate(""2025-09-26""))")
    TestUtil.AssertEqual(Format(#2025-09-26#), "09/26/2025", "Format(#2025-09-26#)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testFormat", Err, Error$, Erl)
End Sub
