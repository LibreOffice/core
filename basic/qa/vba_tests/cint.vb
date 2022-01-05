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
    verify_testCInt
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCInt()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(CInt(-1.1), -1, "CInt(-1.1)")
    TestUtil.AssertEqual(CInt(-1.1), -1, "CInt(-1.1)")
    TestUtil.AssertEqual(CInt(-1.9), -2, "CInt(-1.9)")
    TestUtil.AssertEqual(CInt(0.2),   0, "CInt(0.2)")

REM In excel:
REM    If the fraction is less than or equal to .5, the result will round down.
REM    If the fraction is greater than .5, the result will round up.

REM    TestUtil.AssertEqual(CInt(0.5), 0, "CInt(0.5)")
REM    TestUtil.AssertEqual(CInt(1.5), 2, "CInt(1.5)")
REM    TestUtil.AssertEqual(CInt(2.5), 2, "CInt(2.5)")

    TestUtil.AssertEqual(CInt(10.51),       11, "CInt(10.51)")
    TestUtil.AssertEqual(CInt("&H75FF"), 30207, "CInt(""&H75FF"")")
    TestUtil.AssertEqual(CInt("&H754"),   1876, "CInt(""&H754"")")
    TestUtil.AssertEqual(CInt("+21"),       21, "CInt(""+21"")")

    TestUtil.AssertEqual(CInt(-258.0427e+2 + "133"*2 + &h64 -&o220), -25582, "CInt(-258.0427e+2 + ""133""*2 + &h64 -&o220)")
    TestUtil.AssertEqual(CInt(expression := #1989-09-16#),            32767, "CInt(expression := #1989-09-16#)")
    TestUtil.AssertEqual(CInt(#9/16/1989#),                           32767, "CInt(#9/16/1989#)")

    REM   Dates missing closing dash sign provide unpredictable results
    REM   Dates missing closing dash sign should raise error #13 , as date notation requires it

    TestUtil.AssertEqual(CInt(#1989-09-16),        1964, "CInt(#1989-09-16)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCInt", Err, Error$, Erl)
End Sub
