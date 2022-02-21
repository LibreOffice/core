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
    verify_testIDiv
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testIDiv
    On Error GoTo errorHandler

    ' tdf#147089 - IDIV operands are rounded to Integer values before the operation is performed
    TestUtil.AssertEqual(16.4 \ 5.9, 2, "16.4 \ 5.9")
    TestUtil.AssertEqual(15.9 \ 6.4, 2, "15.9 \ 6.4")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testIDiv", Err, Error$, Erl)
End Sub
