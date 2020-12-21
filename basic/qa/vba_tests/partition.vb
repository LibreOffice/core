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
    verify_testPartition
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testPartition()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(Partition(20, 0, 98, 5),  "20:24",   "Partition(20, 0, 98, 5)")
    TestUtil.AssertEqual(Partition(20, 0, 99, 1),  " 20: 20", "Partition(20, 0, 99, 1)")
    TestUtil.AssertEqual(Partition(120, 0, 99, 5), "100:   ", "Partition(120, 0, 99, 5)")
    TestUtil.AssertEqual(Partition(-5, 0, 99, 5),  "   : -1", "Partition(-5, 0, 99, 5)")
    TestUtil.AssertEqual(Partition(2, 0, 5, 2),    " 2: 3",   "Partition(2, 0, 5, 2)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testPartition", Err, Error$, Erl)
End Sub
