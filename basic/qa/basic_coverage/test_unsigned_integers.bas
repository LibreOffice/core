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
    verify_testUnsignedIntegers
    doUnitTest = TestUtil.GetResult()
End Function

Function dec2hex(n) As String
    Dim fa As Object
    fa = CreateUnoService("com.sun.star.sheet.FunctionAccess")
    dec2hex = fa.callFunction("DEC2HEX", Array(n))
End Function

Sub verify_testUnsignedIntegers()
    On Error GoTo errorHandler

    Dim t As New com.sun.star.util.Time ' has both unsigned long and unsigned short
    t.Seconds = 201
    t.NanoSeconds = 202
    Dim u8 As Byte, u16, u32
    u8 = 200
    u16 = t.Seconds ' UShort
    u32 = t.NanoSeconds ' ULong

    TestUtil.AssertEqual(TypeName(u8), "Byte", "TypeName(u8)")
    TestUtil.AssertEqual(dec2hex(u8), "C8",  "dec2hex(u8)")

    TestUtil.AssertEqual(TypeName(u16), "UShort", "TypeName(u16)")
    TestUtil.AssertEqual(dec2hex(u16), "C9", "dec2hex(u16)")

    TestUtil.AssertEqual(TypeName(u32), "ULong", "TypeName(u32)")
    TestUtil.AssertEqual(dec2hex(u32), "CA", "dec2hex(u32)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testUnsignedIntegers", Err, Error$, Erl)
End Sub
