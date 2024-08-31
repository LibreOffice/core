'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
    TestUtil.TestInit
    verify_testCCurZhCNLocale
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCCurZhCNLocale
    On Error GoTo errorHandler

    ' tdf#141050 - characteristic test for CCur() with the zh_CN locale
    TestUtil.AssertEqual(CCur("75.50"), 75.5, "CCur(75.50)")
    TestUtil.AssertEqual(CCur("￥75.50"), 75.5, "CCur(￥75.50)")
    ' tdf#162724 - CStr must create strings that allow CCur round-trip
    TestUtil.AssertEqual(CCur(CStr(CCur(75.50))), 75.5, "CCur(CStr(CCur(75.50)))")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCCurZhCNLocale", Err, Error$, Erl)
End Sub
