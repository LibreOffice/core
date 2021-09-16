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
    verify_testErrorMessage
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testErrorMessage()

try: On Error Goto catch

    a = 5

catch:

    ' tdf#123144 - check for a meaningful error message
    ' Without the fix in place, this test would have failed with
    ' - Expected: Variable not defined.\n Additional information: a
    ' - Actual  : a
    TestUtil.AssertEqual(Err.Description, _
        + "Variable not defined." & Chr$(10) & "Additional information: a", _
        + "Err.Description failure (Err.Description = " & Err.Description & ")")

End Sub
