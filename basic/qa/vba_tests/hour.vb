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
    verify_testHour
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testHour()
    On Error GoTo errorHandler
    
    TestUtil.AssertEqual(Hour("6:25:39 AM"),  6, "Hour(""6:25:39 AM"")")
    TestUtil.AssertEqual(Hour("6:25:39 PM"), 18, "Hour(""6:25:39 PM"")")
    TestUtil.AssertEqual(Hour("06:25:39 AM"), 6, "Hour(""06:25:39 AM"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testHour", Err, Error$, Erl)
End Sub
