' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testTransliteration
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testTransliteration
    On Error GoTo errorHandler

    Dim oTr As Object, s$, oLoc as new com.sun.star.lang.Locale

    With oLoc
        .Country="US"
        .Language="en"
        .Variant=""
    End With

    s="AB cd. eF. "
    oTr = CreateUnoService("com.sun.star.i18n.Transliteration")

    oTr.LoadModuleByImplName("LOWERCASE_UPPERCASE", oLoc)
    TestUtil.AssertEqual(oTr.transliterateString2String(s, 0, Len(s)), "AB CD. EF. ", "LOWERCASE_UPPERCASE")

    oTr.LoadModuleByImplName("UPPERCASE_LOWERCASE", oLoc)
    TestUtil.AssertEqual(oTr.transliterateString2String(s, 0, Len(s)), "ab cd. ef. ", "UPPERCASE_LOWERCASE")

    oTr.LoadModuleByImplName("TOGGLE_CASE", oLoc)
    TestUtil.AssertEqual(oTr.transliterateString2String(s, 0, Len(s)), "ab CD. Ef. ", "TOGGLE_CASE")

    oTr.LoadModuleByImplName("IGNORE_CASE", oLoc)
    TestUtil.AssertEqual(oTr.transliterateString2String(s, 0, Len(s)), "ab cd. ef. ", "IGNORE_CASE")

    ' tdf#152520: Without the fix in place, this test would have crashed here
    oTr.LoadModuleByImplName("TITLE_CASE", oLoc)
    TestUtil.AssertEqual(oTr.transliterateString2String(s, 0, Len(s)), "Ab cd. ef. ", "TITLE_CASE")

    oTr.LoadModuleByImplName("SENTENCE_CASE", oLoc)
    TestUtil.AssertEqual(oTr.transliterateString2String(s, 0, Len(s)), "Ab cd. ef. ", "SENTENCE_CASE")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testTransliteration", Err, Error$, Erl)
End Sub
