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
    verify_testFormatNumber
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testFormatNumber()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(FormatNumber("12.2", 2, vbFalse, vbFalse, vbFalse),                "12.20",                   "FormatNumber(""12.2"", 2, vbFalse, vbFalse, vbFalse)")
    TestUtil.AssertEqual(FormatNumber("-.2", 20, vbTrue, vbFalse, vbFalse),                 "-0.20000000000000000000", "FormatNumber(""-.2"", 20, vbTrue, vbFalse, vbFalse)")
    TestUtil.AssertEqual(FormatNumber("-0.2", 20, vbFalse, vbFalse, vbFalse),               "-.20000000000000000000",  "FormatNumber(""-0.2"", 20, vbFalse, vbFalse, vbFalse)")
    TestUtil.AssertEqual(FormatNumber("-0.2", -1, vbFalse, vbTrue, vbFalse),                "(.20)",                   "FormatNumber(""-0.2"", -1, vbFalse, vbTrue, vbFalse)")
    TestUtil.AssertEqual(FormatNumber("-0.2", -1, vbUseDefault, vbTrue, vbFalse),           "(0.20)",                  "FormatNumber(""-0.2"", -1, vbUseDefault, vbTrue, vbFalse)")
    TestUtil.AssertEqual(FormatNumber("-12345678", -1, vbUseDefault, vbUseDefault, vbTrue), "-12,345,678.00",          "FormatNumber(""-12345678"", -1, vbUseDefault, vbUseDefault, vbTrue)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testFormatNumber", Err, Error$, Erl)
End Sub
