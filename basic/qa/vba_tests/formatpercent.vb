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
    verify_testFormatPercent
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testFormatPercent()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(FormatPercent("12.2", 2, vbFalse, vbFalse, vbFalse),                "1220.00%",            "FormatPercent(""12.2"", 2, vbFalse, vbFalse, vbFalse)")
    TestUtil.AssertEqual(FormatPercent("-.2", 2, vbTrue, vbFalse, vbFalse),                 "-20.00%",              "FormatPercent(""-.2"", 20, vbTrue, vbFalse, vbFalse)")
    TestUtil.AssertEqual(FormatPercent("-0.2", 2, vbFalse, vbFalse, vbFalse),               "-20.00%",              "FormatPercent(""-0.2"", 20, vbFalse, vbFalse, vbFalse)")
    TestUtil.AssertEqual(FormatPercent("-0.2", -1, vbFalse, vbTrue, vbFalse),                "(20.00)%",            "FormatPercent(""-0.2"", -1, vbFalse, vbTrue, vbFalse)")
    TestUtil.AssertEqual(FormatPercent("-0.2", -1, vbUseDefault, vbTrue, vbFalse),           "(20.00)%",            "FormatPercent(""-0.2"", -1, vbUseDefault, vbTrue, vbFalse)")
    TestUtil.AssertEqual(FormatPercent("-12345678", -1, vbUseDefault, vbUseDefault, vbTrue), "-1,234,567,800.00%",  "FormatPercent(""-12345678"", -1, vbUseDefault, vbUseDefault, vbTrue)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testFormatPercent", Err, Error$, Erl)
End Sub
