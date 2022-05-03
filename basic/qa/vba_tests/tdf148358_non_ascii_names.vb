'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Option Explicit

Function doUnitTest as String
  TestUtil.TestInit
  verify_testNonAsciiNames
  doUnitTest = TestUtil.GetResult()
End Function

Function TestNonAscii as Integer
    Dim Абв as Integer
    Абв = 10
    TestNonAscii = абв
End Function

Sub verify_testNonAsciiNames
  On Error GoTo errorHandler

  ' tdf#148358 - compare Non-ASCII variable names case-insensitive
  TestUtil.AssertEqual(TestNonAscii(), 10, "TestNonAscii()")

  Exit Sub
errorHandler:
  TestUtil.ReportErrorHandler("verify_testNonAsciiNames", Err, Error$, Erl)
End Sub
