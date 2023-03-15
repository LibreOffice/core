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
    verify_testStrConv
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testStrConv()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(StrConv("abc EFG hij αβγ ΔΕΖ ηθι", vbUpperCase),  "ABC EFG HIJ ΑΒΓ ΔΕΖ ΗΘΙ", "StrConv(""abc EFG hij αβγ ΔΕΖ ηθι"", vbUpperCase)")
    TestUtil.AssertEqual(StrConv("abc EFG hij αβγ ΔΕΖ ηθι", vbLowerCase),  "abc efg hij αβγ δεζ ηθι", "StrConv(""abc EFG hij αβγ ΔΕΖ ηθι"", vbLowerCase)")
    TestUtil.AssertEqual(StrConv("abc EFG hij αβγ ΔΕΖ ηθι", vbProperCase), "Abc Efg Hij Αβγ Δεζ Ηθι", "StrConv(""abc EFG hij αβγ ΔΕΖ ηθι"", vbProperCase)")

    ' Converts narrow (single-byte) characters in string to wide
    TestUtil.AssertEqual(StrConv("ABCDEVB¥ì¥¹¥­¥å©", vbWide), "ＡＢＣＤＥＶＢ￥ì￥¹￥­￥å©", "StrConv(""ABCDEVB¥ì¥¹¥­¥å©"", vbWide)")
    TestUtil.AssertEqual(StrConv("ABCDEVB¥ì¥¹¥­¥å©", vbWide + vbLowerCase), "ａｂｃｄｅｖｂ￥ì￥¹￥­￥å©", "StrConv(""ABCDEVB¥ì¥¹¥­¥å©"", vbWide + vbLowerCase)")

    ' Converts wide (double-byte) characters in string to narrow (single-byte) characters
    TestUtil.AssertEqual(StrConv("ＡＢＣＤ＠＄％２３＇？ＥＧ", vbNarrow), "ABCD@$%23'?EG", "StrConv(""ＡＢＣＤ＠＄％２３＇？ＥＧ"", vbNarrow)")
    TestUtil.AssertEqual(StrConv("ＡＢＣＤ＠＄％２３＇？ＥＧ", vbNarrow + vbLowerCase), "abcd@$%23'?eg", "StrConv(""ＡＢＣＤ＠＄％２３＇？ＥＧ"", vbNarrow + vbLowerCase)")

    ' Converts Hiragana characters in string to Katakana characters
    TestUtil.AssertEqual(StrConv("かたかな", vbKatakana), "カタカナ", "StrConv(""かたかな"", vbKatakana)")
    TestUtil.AssertEqual(StrConv("かたかな abc", vbKatakana + vbUpperCase + vbWide), "カタカナ　ＡＢＣ", "StrConv(""かたかな abc"", vbKatakana + vbUpperCase + vbWide)")

    ' Converts Katakana characters in string to Hiragana characters
    TestUtil.AssertEqual(StrConv("カタカナ", vbHiragana), "かたかな", "StrConv(""カタカナ"", vbHiragana)")
    TestUtil.AssertEqual(StrConv("カタカナ　ＡＢＣ", vbLowerCase + vbNarrow), "ｶﾀｶﾅ abc", "StrConv(""カタカナ　ＡＢＣ"", vbLowerCase + vbNarrow)")

    Dim x() As Byte
    Const Cp1252TestString = "ÉÏº£ÊÐABC"

    x = StrConv(Cp1252TestString, vbFromUnicode, &h0409)' CP-1252 encoding associated with en-US locale
    TestUtil.AssertEqual(UBound(x), 8, "UBound(x)")
    TestUtil.AssertEqual(x(0), 201, "x(0)")
    TestUtil.AssertEqual(x(1), 207, "x(1)")
    TestUtil.AssertEqual(x(2), 186, "x(2)")
    TestUtil.AssertEqual(x(3), 163, "x(3)")
    TestUtil.AssertEqual(x(4), 202, "x(4)")
    TestUtil.AssertEqual(x(5), 208, "x(5)")
    TestUtil.AssertEqual(x(6), 65, "x(6)")
    TestUtil.AssertEqual(x(7), 66, "x(7)")
    TestUtil.AssertEqual(x(8), 67, "x(8)")
    TestUtil.AssertEqual(StrConv(x, vbUnicode, &h0409), Cp1252TestString, "StrConv(x, vbUnicode, &h0409)")

    x = StrConv(Cp1252TestString, vbUnicode, &h0409)
    TestUtil.AssertEqual(UBound(x), 35, "UBound(x)")
    TestUtil.AssertEqual(StrConv(x, vbFromUnicode, &h0409), Cp1252TestString, "StrConv(x, vbFromUnicode, &h0409)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testStrConv", Err, Error$, Erl)
End Sub
