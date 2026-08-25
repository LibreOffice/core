'
' This file is part of the Collabora Office project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testCompat
    doUnitTest = TestUtil.GetResult()
End Function

Function convertToDouble(n)
    Dim conv As Object
    conv = CreateUnoService("com.sun.star.script.Converter")
    convertToDouble = conv.convertToSimpleType(n, com.sun.star.uno.TypeClass.DOUBLE)
End Function

Sub verify_testCompat()
    On Error GoTo errorHandler

    Dim u8 As Byte
    u8 = 200
    TestUtil.AssertEqual(convertToDouble(u8), 200, "convertToDouble(u8)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCompat", Err, Error$, Erl)
End Sub
