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
    verify_tdf134692
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_tdf134692()
    On Error GoTo errorHandler

    dim a(2 to 5)
    dim b, i, s
    b = a
    redim preserve b(4 to 6)
    for i = lbound(b) to ubound(b)
      b(i) = i
    next i
    for i = lbound(a) to ubound(a)
      s = s & " a(" & i & ")=" & a(i)
    next i
    for i = lbound(b) to ubound(b)
      s = s & " b(" & i & ")=" & b(i)
    next i
    TestUtil.AssertEqual(s, " a(2)= a(3)= a(4)= a(5)= b(4)=4 b(5)=5 b(6)=6", "tdf#134692")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_tdf134692", Err, Error$, Erl)
End Sub
