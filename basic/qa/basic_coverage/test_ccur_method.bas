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
    verify_testCCur
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCCur
    On Error GoTo errorHandler

    ' CCUR
    TestUtil.AssertEqual(CCur("100"), 100, "CCur(100)")

    ' tdf#141050 - passing a number with + sign
    TestUtil.AssertEqual(CCur("+100"), 100, "CCur(100)")
    ' tdf#141050 - passing a number with - sign
    TestUtil.AssertEqual(CCur("-100"), -100, "CCur(-100)")

    ' tdf#128122 - verify en_US locale currency format behavior
    TestUtil.AssertEqual(CCur("$100"), 100, "CCur($100)")
    TestUtil.AssertEqual(CCur("$1.50"), 1.5, "CCur($1.50)")

    ' tdf#162724 - CStr must create strings that allow CCur round-trip
    TestUtil.AssertEqual(CCur(CStr(CCur(75.50))), 75.5, "CCur(CStr(CCur(75.50)))")

    verify_testCCurUnderflow
    verify_testCCurOverflow
    verify_testCCurInvalidFormat

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCCur", Err, Error$, Erl)
End Sub

sub verify_testCCurUnderflow
    On Error GoTo underflowHandler

    ' tdf$128122 - test underflow condition
    CCur("-9223372036854775809")
    TestUtil.Assert(False, "verify_testCCur", "underflow error not raised")

    Exit Sub
underflowHandler:
    If(Err <> 6) Then
        TestUtil.Assert(False, "verify_testCCur", "underflow error incorrect type")
    Endif
End Sub

sub verify_testCCurOverflow
    On Error GoTo overflowHandler

    ' tdf$128122 - test overflow condition
    CCur("9223372036854775808")
    TestUtil.Assert(False, "verify_testCCur", "overflow error not raised")

    Exit Sub
overflowHandler:
    If(Err <> 6) Then
        TestUtil.Assert(False, "verify_testCCur", "overflow error incorrect type")
    Endif
End Sub

sub verify_testCCurInvalidFormat
    On Error GoTo invalidFormatHandler

    ' tdf$128122 - test invalid format in en_US locale
    CCur("75,50 kr")
    TestUtil.Assert(False, "verify_testCCur", "invalid format error not raised")

    Exit Sub
invalidFormatHandler:
    If(Err <> 13) Then
        TestUtil.Assert(False, "verify_testCCur", "invalid format error incorrect type")
    Endif
End Sub
