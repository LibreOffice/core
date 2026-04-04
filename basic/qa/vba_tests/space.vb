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
    verify_testSpace
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testSpace()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Space(3), "   ", "Space(3)")
    TestUtil.AssertEqual(Space(2), "  ",  "Space(2)")
    TestUtil.AssertEqual(Space(0), "",    "Space(0)")

    ' in VBA it must throw when used with negative values
    TestUtil.AssertEqual(GetNegativeSpaceError(-1), 5, "GetNegativeSpaceError(-1)")
    TestUtil.AssertEqual(GetNegativeSpaceError(-4), 5, "GetNegativeSpaceError(-4)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testSpace", Err, Error$, Erl)
End Sub

Function GetNegativeSpaceError(negativeValue) As Long
On Error GoTo errorHandler
    Space(negativeValue)
    GetNegativeSpaceError = 0
    Exit Function
errorHandler:
    GetNegativeSpaceError = Err
End Function
