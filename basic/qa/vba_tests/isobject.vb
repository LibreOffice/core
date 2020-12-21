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
    verify_testIsObject
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testIsObject()
    On Error GoTo errorHandler
    Dim TestStr As String
    Dim MyObject As Object
    Dim YourObject

    Set YourObject = MyObject    ' Assign an object reference.
    TestUtil.Assert(IsObject(YourObject),  "IsObject(YourObject)")
    TestUtil.Assert(Not IsObject(TestStr), "Not IsObject(TestStr)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testIsObject", Err, Error$, Erl)
End Sub
