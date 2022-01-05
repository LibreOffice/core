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
    verify_CInt
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_CInt()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(CInt("103.15"),  103,  "CInt(""103.15"")")

    ' strings are rejected in complex expressions, while VBA mode accepts them
    ' VBA converts: -258.0427e+2 + "133"*2 + &h64 -&o220 
    TestUtil.AssertEqual(CInt(-258.0427e+2 + 133*2 + &h64 -&o220), -25582, "CInt(-258.0427e+2 + 133*2 + &h64 -&o220)")
    TestUtil.AssertEqual(CInt(expression := #1990-10-27),            1953, "CInt(expression := #1990-10-27)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_CInt", Err, Error$, Erl)
End Sub

