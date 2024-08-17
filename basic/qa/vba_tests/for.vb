'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1

Function doUnitTest() As String
    TestUtil.TestInit
    test_for
    doUnitTest = TestUtil.GetResult()
End Function

Sub test_for()
    On Error GoTo errorHandler

    Dim i As Long, j As Long, k As Long
    ' Test that loop end value is evaluated only once,
    ' and then changing the variable doesn't affect it
    k=10
    For i=1 To k
      k=1
      j=i
    Next i
    TestUtil.AssertEqual(j, 10, "Last loop control value (changing end variable)")

    ' Test that loop step value is evaluated only once,
    ' and then changing the variable doesn't affect it
    k=1
    j=0
    For i=1 To 10 Step k
      k=10
      j=j+1
    Next i
    TestUtil.AssertEqual(j, 10, "Number of iterations (changing step variable)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testOptionalsVba", Err, Error$, Erl)
End Sub
