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
    verify_testCDec
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCDec()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(CDec(""),             0, "CDec("""")")
    TestUtil.AssertEqual(CDec("1234"),      1234, "CDec(""1234"")")
    TestUtil.AssertEqual(CDec("  1234  "),  1234, "CDec(""  1234  "")")
    TestUtil.AssertEqual(CDec("-1234"),    -1234, "CDec(""-1234"")")
    TestUtil.AssertEqual(CDec(" - 1234 "), -1234, "CDec("" - 1234 "")")

    '''''''''''''''
    ' Those are erroneous, see i#64348
    TestUtil.AssertEqual(CDec("1234-"),     -1234, "CDec(""1234-"")")
    TestUtil.AssertEqual(CDec("  1234  -"), -1234, "CDec(""  1234  -"")")

    'TestUtil.AssertEqual(CDec("79228162514264300000000000001"),     79228162514264300000000000001, "CDec(""79228162514264300000000000001"")")
    'TestUtil.AssertEqual(CDec("79228162514264300000000000001") + 1, 79228162514264300000000000002, "CDec(""79228162514264300000000000001"") + 1")

    TestUtil.AssertEqual(CDec("79228162514264400000000000000"), 62406456049664, "CDec(""79228162514264400000000000000"")")
    TestUtil.AssertEqual(CDec("79228162514264340000000000000"),              0, "CDec(""79228162514264340000000000000"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCDec", Err, Error$, Erl)
End Sub
