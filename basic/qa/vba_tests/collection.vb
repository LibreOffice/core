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
    verify_testCollection
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCollection()

    Dim c As Collection

    On Error Resume Next
    Set c = New Collection
    c.Add 1, "D"
    c.Add 2, "d"
    c.Add 3, "Д"
    c.Add 4, "д"
    On Error GoTo 0

    On Error GoTo errorHandler

    ' tdf#144245 - check correct initialization of default value for optionals
    ' Without the fix in place, this test would have failed with
    ' - Expected: 2
    ' - Actual  : 3
    TestUtil.AssertEqual(c.Count, 2, "c.Count")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCollection", Err, Error$, Erl)
End Sub
