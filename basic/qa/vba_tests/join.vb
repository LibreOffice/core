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
    verify_testJoin
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testJoin()
    On Error GoTo errorHandler
    Dim vaArray(2) As String
    vaArray(0) = "string1"
    vaArray(1) = "string2"
    vaArray(2) = "string3"

    TestUtil.AssertEqual(Join(vaArray),       "string1 string2 string3",   "Join(vaArray)")
    TestUtil.AssertEqual(Join(vaArray, " "),  "string1 string2 string3",   "Join(vaArray, "" "")")
    TestUtil.AssertEqual(Join(vaArray, "<>"), "string1<>string2<>string3", "Join(vaArray, ""<>"")")
    TestUtil.AssertEqual(Join(vaArray, ""),   "string1string2string3",     "Join(vaArray, """")")

    ' tdf#141474 keyword names need to match that of VBA
    Dim aList(0 to 7) As String : aList = Array("(", "Star", "|", "Open", "|", "Libre", ")", "Office")
    TestUtil.AssertEqual(Join(sourceArray:=aList), "( Star | Open | Libre ) Office", "Join() with 1 keyword name")
    TestUtil.AssertEqual(Join(delimiter:="", sourceArray:=aList), "(Star|Open|Libre)Office", "Join() with 2 keyword names")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testJoin", Err, Error$, Erl)
End Sub
