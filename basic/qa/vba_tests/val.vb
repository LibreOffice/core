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
    verify_testVal
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testVal()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Val("02/04/2010"),             2,    "Val(""02/04/2010"")")
    TestUtil.AssertEqual(Val("1050"),                1050,    "Val(""1050"")")
    TestUtil.AssertEqual(Val("130.75"),               130.75, "Val(""130.75"")")
    TestUtil.AssertEqual(Val("50 Park Lane"),          50,    "Val(""50 Park Lane"")")
    TestUtil.AssertEqual(Val("1320 then some text"), 1320,    "Val(""1320 then some text"")")
    TestUtil.AssertEqual(Val("L13.5"),                  0,    "Val(""L13.5"")")
    TestUtil.AssertEqual(Val("sometext"),               0,    "Val(""sometext"")")
REM    tdf#111999
REM    TestUtil.AssertEqual(Val("1, 2"),                   1,    "Val(""1, 2"")")
    TestUtil.AssertEqual(Val("&HFFFF"),                -1,    "Val(""&HFFFF"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testVal", Err, Error$, Erl)
End Sub
