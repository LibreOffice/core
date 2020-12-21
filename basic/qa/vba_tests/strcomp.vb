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
    verify_testSTRcomp
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testSTRcomp()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(StrComp("ABCD",   "abcd", vbTextCompare),    0, "StrComp(""ABCD"", ""abcd"", vbTextCompare)")
    TestUtil.AssertEqual(StrComp("ABCD",   "abcd", vbBinaryCompare), -1, "StrComp(""ABCD"", ""abcd"", vbBinaryCompare)")
    TestUtil.AssertEqual(StrComp("ABCD",   "abcd"),                  -1, "StrComp(""ABCD"", ""abcd"")")
    TestUtil.AssertEqual(StrComp("text",   "text", vbBinaryCompare),  0, "StrComp(""text"", ""text"", vbBinaryCompare)")
    TestUtil.AssertEqual(StrComp("text  ", "text", vbBinaryCompare),  1, "StrComp(""text  "", ""text"", vbBinaryCompare)")
    TestUtil.AssertEqual(StrComp("Text",   "text", vbBinaryCompare), -1, "StrComp(""Text"", ""text"", vbBinaryCompare)")
    TestUtil.AssertEqual(StrComp("text",   "text", vbTextCompare),    0, "StrComp(""text"", ""text"", vbTextCompare)")
    TestUtil.AssertEqual(StrComp("text  ", "text", vbTextCompare),    1, "StrComp(""text  "", ""text"", vbTextCompare)")
    TestUtil.AssertEqual(StrComp("Text",   "text", vbTextCompare),    0, "StrComp(""Text"", ""text"", vbTextCompare)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testSTRcomp", Err, Error$, Erl)
End Sub
