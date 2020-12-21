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
    verify_testIsEmpty
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testIsEmpty()
    On Error GoTo errorHandler

    Dim MyVar
    TestUtil.Assert(IsEmpty(MyVar),     "IsEmpty(MyVar)")

    MyVar = Null    ' Assign Null.
    TestUtil.Assert(Not IsEmpty(MyVar), "Not IsEmpty(MyVar)")

    MyVar = Empty    ' Assign Empty.
    TestUtil.Assert(IsEmpty(MyVar),     "IsEmpty(MyVar)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testIsEmpty", Err, Error$, Erl)
End Sub
