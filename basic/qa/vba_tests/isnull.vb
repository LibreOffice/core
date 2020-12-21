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
    verify_testIsNull
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testIsNull()
    On Error GoTo errorHandler

    TestUtil.Assert(IsNull(Null),   "IsNull(Null)")
    TestUtil.Assert(Not IsNull(""), "Not IsNull("""")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testIsNull", Err, Error$, Erl)
End Sub
