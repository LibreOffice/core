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
    verify_testTypeNameBoolean
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testTypeNameBoolean()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(TypeName 1>2,       "Boolean",  "TypeName 1>2")
    TestUtil.AssertEqual(TypeName 2.0>1.0,   "Boolean",  "TypeName 2.0>1.0")
    TestUtil.AssertEqual(TypeName "A">"B",   "Boolean",  "TypeName ""A"">""B""")

    TestUtil.AssertEqual(Str(2>1), "True",  "Str(2>1)")
    TestUtil.AssertEqual(Str(1>2), "False", "Str(1>2)")

    TestUtil.AssertEqual(Str(2.0>1.0), "True",  "Str(2.0>1.0)")
    TestUtil.AssertEqual(Str(1.0>2.0), "False", "Str(1.0>2.0)")

    TestUtil.AssertEqual(Str("B">"A"), "True",  "Str(""B"">""A"")")
    TestUtil.AssertEqual(Str("A">"B"), "False", "Str(""A"">""B"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testTypeNameBoolean", Err, Error$, Erl)
End Sub
