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

    TestUtil.AssertEqual(StrConv("abc EFG hij", vbUpperCase),  "ABC EFG HIJ", "StrConv(""abc EFG hij"", vbUpperCase)")
    TestUtil.AssertEqual(StrConv("abc EFG hij", vbLowerCase),  "abc efg hij", "StrConv(""abc EFG hij"", vbLowerCase)")
    TestUtil.AssertEqual(StrConv("abc EFG hij", vbProperCase), "Abc Efg Hij", "StrConv(""abc EFG hij"", vbProperCase)")
    
    ' Converts narrow (single-byte) characters in string to wide
    TestUtil.AssertEqual(StrConv("ABCDEVB¥ì¥¹¥­¥å©", vbWide), "ＡＢＣＤＥＶＢ￥ì￥¹￥­￥å©", "StrConv(""ABCDEVB¥ì¥¹¥­¥å©"", vbWide)")

    ' Converts wide (double-byte) characters in string to narrow (single-byte) characters
    TestUtil.AssertEqual(StrConv("£Á£Â£Ã£Ä£ÅVB¥ì¥¹¥­¥å©`", vbNarrow), "£Á£Â£Ã£Ä£ÅVB¥ì¥¹¥­¥å©`", "StrConv(""£Á£Â£Ã£Ä£ÅVB¥ì¥¹¥­¥å©`"", vbNarrow)")

    ' Converts Hiragana characters in string to Katakana characters
    TestUtil.AssertEqual(StrConv("¤Ï¤Ê¤Á¤ã¤ó", vbKatakana, 1041), "¤Ï¤Ê¤Á¤ã¤ó", "StrConv(""¤Ï¤Ê¤Á¤ã¤ó"", vbKatakana)")

    ' Converts Katakana characters in string to Hiragana characters
    TestUtil.AssertEqual(StrConv("¥Ï¥Ê¥Á¥ã¥ó", vbHiragana, 1041), "¥Ï¥Ê¥Á¥ã¥ó", "StrConv(""¥Ï¥Ê¥Á¥ã¥ó"", vbHiragana)")

    Dim x() As Byte
    x = StrConv("ÉÏº£ÊÐABC", vbFromUnicode)
    if GetGUIType() = 4 then
        TestUtil.AssertEqual(UBound(x), 14, "UBound(x)")
    Else
        TestUtil.AssertEqual(UBound(x), 8, "UBound(x)")
    End if
    TestUtil.AssertEqual(StrConv(x, vbUnicode), "ÉÏº£ÊÐABC", "StrConv(x, vbUnicode)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testStrConv", Err, Error$, Erl)
End Sub
