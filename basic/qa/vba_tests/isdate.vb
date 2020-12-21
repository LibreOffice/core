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
    verify_testIsDate
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testIsDate()
    On Error GoTo errorHandler

    TestUtil.Assert(IsDate(cdate("12/2/1969")), "IsDate(cdate(""12/2/1969""))")
    TestUtil.Assert(IsDate("12:22:12"),         "IsDate(""12:22:12"")")
    TestUtil.Assert(Not IsDate("a12.2.1969"),   "Not IsDate(""a12.2.1969"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testIsDate", Err, Error$, Erl)
End Sub
