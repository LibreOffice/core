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
    verify_ByteArrayString
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_ByteArrayString()
    Dim MyString As String
    Dim x() As Byte

    On Error GoTo errorHandler

    MyString = "abc"
    x = MyString ' string -> byte array

    ' test bytes in string
    TestUtil.AssertEqual(UBound(x), 5, "UBound(x)")

    MyString = x 'byte array -> string
    TestUtil.AssertEqual(MyString, "abc", "MyString")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_ByteArrayString", Err, Error$, Erl)
End Sub
