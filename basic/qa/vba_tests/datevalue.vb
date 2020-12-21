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
    verify_testDateValue
    doUnitTest = TestUtil.GetResult()
End Function

Function verify_testDateValue() as String
    On Error GoTo errorHandler

    TestUtil.AssertEqual(DateValue("February 12, 1969"), 25246, "DateValue(""February 12, 1969"")")
    TestUtil.AssertEqual(DateValue("21/01/2008"),        39468, "DateValue(""21/01/2008"")")

    Exit Function
errorHandler:
    TestUtil.ReportErrorHandler("verify_testFix", Err, Error$, Erl)
End Sub
