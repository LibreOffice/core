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
    verify_GoSub_GoTo
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_GoSub_GoTo
    On Error GoTo errorHandler

    Dim iVar As Integer

    ' tdf#160321 - don't execute the GoSub statement if the expression is 0
    On 0 GoSub Sub1, Sub2
    iVar = iVar + 1
    TestUtil.AssertEqual(iVar, 1, "iVar incremented incorrectly")

    ' tdf#160321 - check the correct functionality of the GoTo statement
    On 1 GoTo Sub1, Sub2
    iVar = iVar + 1

    Exit Sub
Sub1:
    TestUtil.AssertEqual(iVar, 1, "iVar incremented incorrectly")
    On 2 GoTo Sub1, Sub2
    iVar = iVar + 1
    Exit Sub
Sub2:
    TestUtil.AssertEqual(iVar, 1, "iVar incremented incorrectly")
    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_GoSub_GoTo", Err, Error$, Erl)
    Exit Sub
End Sub
