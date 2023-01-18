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
    verify_String
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_String()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(String(5, "P"), "PPPPP", "String(5, ""P"")")
    TestUtil.AssertEqual(String(5, "a"), "aaaaa", "String(5, ""a"")")
    TestUtil.AssertEqual(String(0, "P"), "",      "String(0, ""P"")")

    TestUtil.AssertEqual(String(5.8, "à"),                     "àààààà",  "String(5.8, ""à"")")
    TestUtil.AssertEqual(String(Number:=3.45, Character:="test"), "ttt",  "String(Number:=3.45, Character:=""test"")")
    TestUtil.AssertEqual(String(Character:="☺😎", Number:=7), "☺☺☺☺☺☺☺",  "String(Character:=""☺😎"", Number:=7)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_String", Err, Error$, Erl)
End Sub
