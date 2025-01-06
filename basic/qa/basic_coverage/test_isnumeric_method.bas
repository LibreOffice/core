'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_IsNumeric
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_IsNumeric
    On Error GoTo errorHandler

    dim aVariant as Variant
    aVariant = 3
    TestUtil.Assert(IsNumeric(aVariant), "IsNumeric(aVariant)")

    TestUtil.Assert(IsNumeric(" 0 "), "IsNumeric("" 0 "")")
    TestUtil.Assert(IsNumeric(" +0 "), "IsNumeric("" +0 "")")
    TestUtil.Assert(IsNumeric(" -0 "), "IsNumeric("" -0 "")")
    TestUtil.Assert(Not IsNumeric(""), "Not IsNumeric("""")")
    TestUtil.Assert(Not IsNumeric(" "), "Not IsNumeric("" "")")
    TestUtil.Assert(Not IsNumeric(" + "), "Not IsNumeric("" + "")")
    TestUtil.Assert(Not IsNumeric(" - "), "Not IsNumeric("" - "")")
    ' Note: the two following tests behave different in VBA;
    ' should it be unified maybe in non-VBA, too (a breaking change)?
    TestUtil.Assert(Not IsNumeric(" + 0 "), "Not IsNumeric("" + 0 "")")
    TestUtil.Assert(Not IsNumeric(" - 0 "), "Not IsNumeric("" - 0 "")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_IsNumeric", Err, Error$, Erl)
End Sub
