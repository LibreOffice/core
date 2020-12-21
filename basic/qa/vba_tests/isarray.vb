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
    verify_testIsArray
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testIsArray()
    On Error GoTo errorHandler
    Dim MyArray(1 To 5) As Integer, YourArray    ' Declare array variables.
    Dim AVar
    YourArray = Array(1, 2, 3)    ' Use Array function.
    AVar = False

    TestUtil.Assert(IsArray(MyArray),   "IsArray(MyArray)")
    TestUtil.Assert(IsArray(YourArray), "IsArray(YourArray)")
    TestUtil.Assert(Not IsArray(AVar),  "Not IsArray(AVar)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testIsArray", Err, Error$, Erl)
End Sub
