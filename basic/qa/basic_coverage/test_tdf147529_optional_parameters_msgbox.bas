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
    verify_testOptionalParametersMsgBox
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testOptionalParametersMsgBox
    On Error GoTo errorHandler

    ' tdf#147529 - check for missing optional parameters
    TestUtil.AssertEqual(TestOptionalParametersMsgBox(),          True, "TestOptionalParametersMsgBox()")
    TestUtil.AssertEqual(TestOptionalParametersMsgBox("test"),    True, "TestOptionalParametersMsgBox(""test"")")
    TestUtil.AssertEqual(TestOptionalParametersMsgBox("test", 1), True, "TestOptionalParametersMsgBox(""test"", 1)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testOptionalParametersMsgBox", Err, Error$, Erl)
End Sub

Function TestOptionalParametersMsgBox(Optional text, Optional dialogType, Optional dialogTitle) As Boolean
On Error GoTo errorHandler
    MsgBox(text, dialogType, dialogTitle)
    TestOptionalParametersMsgBox = False
    Exit Function
errorHandler:
    TestUtil.AssertEqual(Err, 449, "Argument not optional (Error 449)")
    TestOptionalParametersMsgBox = True
End Function
