'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Rem Attribute VBA_ModuleType=VBAModule
Option VBASupport 1
Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testASC
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testASC()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Asc("A"),     65, "Asc(""A"")")
    TestUtil.AssertEqual(Asc("a"),     97, "Asc(""a"")")
    TestUtil.AssertEqual(Asc("Apple"), 65, "Asc(""Apple"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testASC", Err, Error$, Erl)
End Sub
