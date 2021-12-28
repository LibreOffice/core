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
    verify_testTypeNameBoolean
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testTypeNameBoolean()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(TypeName(1>2),       "Boolean",  "TypeName(1>2)")
    TestUtil.AssertEqual(TypeName(2.0>1.0),   "Boolean",  "TypeName(2.0>1.0)")
    TestUtil.AssertEqual(TypeName("A">"B"),   "Boolean",  "TypeName(""A"">""B"")")

    TestUtil.AssertEqual(Str(2>1), "True",  "Str(2>1)")
    TestUtil.AssertEqual(Str(1>2), "False", "Str(1>2)")

    TestUtil.AssertEqual(Str(2.0>1.0), "True",  "Str(2.0>1.0)")
    TestUtil.AssertEqual(Str(1.0>2.0), "False", "Str(1.0>2.0)")

    TestUtil.AssertEqual(Str("B">"A"), "True",  "Str(""B"">""A"")")
    TestUtil.AssertEqual(Str("A">"B"), "False", "Str(""A"">""B"")")

    ' tdf#145960 - return type of boolean operators should be of type boolean
    TestUtil.AssertEqual(TypeName(True Mod True),   "Long",     "TypeName(True Mod True)")
    TestUtil.AssertEqual(TypeName(True \ True),     "Long",     "TypeName(True \ True)")
    TestUtil.AssertEqual(TypeName(True And True),   "Boolean",  "TypeName(True And True)")
    TestUtil.AssertEqual(TypeName(True Or True),    "Boolean",  "TypeName(True Or True)")
    TestUtil.AssertEqual(TypeName(True Xor True),   "Boolean",  "TypeName(True Xor True)")
    TestUtil.AssertEqual(TypeName(True Eqv True),   "Boolean",  "TypeName(True Eqv True)")
    TestUtil.AssertEqual(TypeName(True Imp True),   "Boolean",  "TypeName(True Imp True)")
    TestUtil.AssertEqual(TypeName(Not True),        "Boolean",  "TypeName(Not True)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testTypeNameBoolean", Err, Error$, Erl)
End Sub
