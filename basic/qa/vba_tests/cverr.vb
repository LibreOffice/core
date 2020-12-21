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
    verify_testCVErr
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCVErr()
    On Error GoTo errorHandler

    TestUtil.AssertEqual(CStr(CVErr(3001)),       "Error 3001", "CStr(CVErr(3001))")
    TestUtil.AssertEqual(CStr(CVErr(xlErrDiv0)),  "Error 2007", "CStr(CVErr(xlErrDiv0))")
    TestUtil.AssertEqual(CStr(CVErr(xlErrNA)),    "Error 2042", "CStr(CVErr(xlErrNA))")
    TestUtil.AssertEqual(CStr(CVErr(xlErrName)),  "Error 2029", "CStr(CVErr(xlErrName))")
    TestUtil.AssertEqual(CStr(CVErr(xlErrNull)),  "Error 2000", "CStr(CVErr(xlErrNull))")
    TestUtil.AssertEqual(CStr(CVErr(xlErrNum)),   "Error 2036", "CStr(CVErr(xlErrNum))")
    TestUtil.AssertEqual(CStr(CVErr(xlErrRef)),   "Error 2023", "CStr(CVErr(xlErrRef))")
    TestUtil.AssertEqual(CStr(CVErr(xlErrValue)), "Error 2015", "CStr(CVErr(xlErrValue))")

    ' tdf#79426 - passing an error object to a function
    TestUtil.AssertEqual(TestCVErr(CVErr(2)),     2, "TestCVErr(CVErr(2))")
    ' tdf#79426 - test with Error-Code 448 ( ERRCODE_BASIC_NAMED_NOT_FOUND )
    TestUtil.AssertEqual(TestCVErr(CVErr(448)), 448, "TestCVErr(CVErr(448))")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCVErr", Err, Error$, Erl)
End Sub

Function TestCVErr(vErr As Variant)
    Dim nValue As Integer
    nValue = vErr
    TestCVErr = nValue
End Function
