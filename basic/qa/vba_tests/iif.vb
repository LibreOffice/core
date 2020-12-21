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
    verify_testIIf
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testIIf()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(IIf(True, "it is true", "it is false"),  "it is true",  "IIf(True, ""it is true"", ""it is false"")")
    TestUtil.AssertEqual(IIf(False, "It is true", "it is false"), "it is false", "IIf(False, ""It is true"", ""it is false"")")

    Dim testnr
    testnr = 1001
    TestUtil.AssertEqual(IIf(testnr > 1000, "Large", "Small"), "Large", "IIf(testnr > 1000, ""Large"", ""Small"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testIIf", Err, Error$, Erl)
End Sub
