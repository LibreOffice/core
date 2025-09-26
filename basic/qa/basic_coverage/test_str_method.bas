' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
    TestUtil.TestInit
    verify_testStr
    doUnitTest = TestUtil.GetResult()
End Function

Dim failedAssertion As Boolean, messages As String

Sub verify_testStr
    On Error GoTo errorHandler

    ' If a string is passed as argument, it is returned without any changes
    TestUtil.AssertEqualStrict(Str(""), "", "Str("""")")
    TestUtil.AssertEqualStrict(Str(" "), " ", "Str("" "")")
    TestUtil.AssertEqualStrict(Str(" 00 "), " 00 ", "Str("" 00 "")")

    ' Non-negative numbers are preceded by a blank space
    TestUtil.AssertEqualStrict(Str(0), " 0", "Str(0)")
    TestUtil.AssertEqualStrict(Str(1 / 10), " 0.1", "Str(1 / 10)")

    ' Negative numbers are preceded by a minus sign
    TestUtil.AssertEqualStrict(Str(-1 / 10), "-0.1", "Str(-1 / 10)")

    ' Dates are converted into locale-dependent strings (test uses en-US)
    TestUtil.AssertEqualStrict(Str(CDate("2025-09-26")), "09/26/2025", "Str(CDate(""2025-09-26""))")
    TestUtil.AssertEqualStrict(Str(#2025-09-26#), "09/26/2025", "Str(#2025-09-26#)")

    TestUtil.AssertEqualStrict(Str(true), "True", "Str(true)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testStr", Err, Error$, Erl)
End Sub
