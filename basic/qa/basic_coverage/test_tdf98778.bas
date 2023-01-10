' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 0
Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_tdf98778
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_tdf98778()
    On Error GoTo errorHandler

    dim d as double
    dim s as string
    s="1.0E-5"
    d=s
    TestUtil.AssertEqual(d, 1E-05, "tdf#98778")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_tdf98778", Err, Error$, Erl)
End Sub
