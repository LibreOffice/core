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
    verify_testIif
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testIif
    On Error GoTo errorHandler

    TestUtil.AssertEqual(IIF(True, 10, 12), 10, "IIF(True, 10, 12)")

    ' tdf#149151
    ' Without the fix, this would fail with "hit error handler - 9: Index out of defined range"
    TestUtil.Assert(IsArray(Iif(True, Array("A","B"), Array("B","A"))), "IsArray(Iif(True, Array(""A"",""B""), Array(""B"",""A"")))")
    TestUtil.AssertEqualStrict(Iif(True, Array("A","B"), Array("B","A"))(0), "A", "Iif(True, Array(""A"",""B""), Array(""B"",""A""))(0)")
    TestUtil.AssertEqualStrict(Iif(True, Array("A","B"), Array("B","A"))(1), "B", "Iif(True, Array(""A"",""B""), Array(""B"",""A""))(1)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testIif", Err, Error$, Erl)
End Sub
