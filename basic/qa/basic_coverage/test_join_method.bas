' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_Join_method
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_Join_method
    On Error GoTo errorHandler

    ' JOIN
    Dim aStrings(2) as String
    aStrings(0) = "Hello"
    aStrings(1) = "world"
    TestUtil.AssertEqual(Join( aStrings, " " ), "Hello world ", "Join(aStrings, "" "" is not ""Hello world """)

    ' tdf#141474 keyword names need to match that of VBA
    Dim aList(0 to 7) As String : aList = Array("(", "Star", "|", "Open", "|", "Libre", ")", "Office")
    TestUtil.AssertEqual(Join(sourceArray:=aList), "( Star | Open | Libre ) Office", "Join() with 1 keyword name")
    TestUtil.AssertEqual(Join(delimiter:="", sourceArray:=aList), "(Star|Open|Libre)Office", "Join() with 2 keyword names")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_Join_method", Err, Error$, Erl)
End Sub

