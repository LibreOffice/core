' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testCurrency
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testCurrency()
    On Error GoTo errorHandler

    Dim x As Currency, y As Currency

    ' Test division

    verify_DivideByZero(15.1@, 0@)
    x = 15.1@
    y = 0.4@
    TestUtil.AssertEqual(x / y, 37.75@, x & " / " & y)
    verify_IDivideByZero(x, y) ' y rounds to 0
    verify_IModByZero(x, y)
    y = 0.8@
    TestUtil.AssertEqual(x / y, 18.875@, x & " / " & y)
    TestUtil.AssertEqual(x \ y, 15, x & " \ " & y) ' y rounds to 1
    TestUtil.AssertEqual(x Mod y, 0, x & " Mod " & y)

    ' Test various operations; the end result is -922337203685477.5808, i.e. minimal representable value
    x = 9223372036.8@
    y = x * 59.000
    x = x * 59.125
    x = x - y
    x = x * 20.48
    x = x / 0.1024
    x = x * 8
    x = x * 0.5
    x = x / 0.001
    Dim [922337203685477.5807 1] As Currency
    [922337203685477.5807 1] = x + 5477.5807
    TestUtil.AssertEqual(CStr([922337203685477.5807 1]), "922337203685477.5807", "CStr([922337203685477.5807 1])")
    Dim [-922337203685477.5808 1] As Currency
    [-922337203685477.5808 1] = -[922337203685477.5807 1] - 0.0001
    TestUtil.AssertEqual(CStr([-922337203685477.5808 1]), "-922337203685477.5808", "CStr([-922337203685477.5808 1])")

    x = -21474836.48@
    y = 42949672.96@
    Dim [-922337203685477.5808 2] As Currency
    [-922337203685477.5808 2] = x * y
    TestUtil.AssertEqual(CStr([-922337203685477.5808 2]), "-922337203685477.5808", "CStr([-922337203685477.5808 2])")

    ' Check huge literals; FIXME: fails yet, because doubles are stored/read in compiler :(
    ' TestUtil.AssertEqualStrict(922337203685477.5807@, [922337203685477.5807 1], "922337203685477.5807@")
    ' x = 922337203685477.5807@
    ' TestUtil.AssertEqualStrict(x, [922337203685477.5807 1], "x = 922337203685477.5807@")

    ' Comparisons
    TestUtil.Assert([-922337203685477.5808 1] = [-922337203685477.5808 2], [-922337203685477.5808 1] & " = " & [-922337203685477.5808 2])
    TestUtil.Assert(Not([-922337203685477.5808 1] <> [-922337203685477.5808 2]), "Not(" & [-922337203685477.5808 1] & " <> " & [-922337203685477.5808 2] & ")")
    TestUtil.Assert(Not([-922337203685477.5808 1] < [-922337203685477.5808 2]), "Not(" & [-922337203685477.5808 1] & " < " & [-922337203685477.5808 2] & ")")
    TestUtil.Assert(Not([-922337203685477.5808 1] > [-922337203685477.5808 2]), "Not(" & [-922337203685477.5808 1] & " > " & [-922337203685477.5808 2] & ")")
    TestUtil.Assert([-922337203685477.5808 1] <= [-922337203685477.5808 2], [-922337203685477.5808 1] & " <= " & [-922337203685477.5808 2])
    TestUtil.Assert([-922337203685477.5808 1] >= [-922337203685477.5808 2], [-922337203685477.5808 1] & " >= " & [-922337203685477.5808 2])

    TestUtil.Assert(Not([-922337203685477.5808 1] = [922337203685477.5807 1]), "Not(" & [-922337203685477.5808 1] & " = " & [922337203685477.5807 1] & ")")
    TestUtil.Assert([-922337203685477.5808 1] <> [922337203685477.5807 1], [-922337203685477.5808 1] & " <> " & [922337203685477.5807 1])
    TestUtil.Assert([-922337203685477.5808 1] < [922337203685477.5807 1], [-922337203685477.5808 1] & " < " & [922337203685477.5807 1])
    TestUtil.Assert(Not([-922337203685477.5808 1] > [922337203685477.5807 1]), "Not(" & [-922337203685477.5808 1] & " > " & [922337203685477.5807 1] & ")")
    TestUtil.Assert([-922337203685477.5808 1] <= [922337203685477.5807 1], [-922337203685477.5808 1] & " <= " & [922337203685477.5807 1])
    TestUtil.Assert(Not([-922337203685477.5808 1] >= [922337203685477.5807 1]), "Not(" & [-922337203685477.5808 1] & " >= " & [922337203685477.5807 1] & ")")

    ' Two close huge negative values
    TestUtil.Assert(Not([-922337203685477.5808 1] = -[922337203685477.5807 1]), "Not(" & [-922337203685477.5808 1] & " = " & -[922337203685477.5807 1] & ")")
    TestUtil.Assert([-922337203685477.5808 1] <> -[922337203685477.5807 1], [-922337203685477.5808 1] & " <> " & -[922337203685477.5807 1])
    TestUtil.Assert([-922337203685477.5808 1] < -[922337203685477.5807 1], [-922337203685477.5808 1] & " < " & -[922337203685477.5807 1])
    TestUtil.Assert(Not([-922337203685477.5808 1] > -[922337203685477.5807 1]), "Not(" & [-922337203685477.5808 1] & " > " & -[922337203685477.5807 1] & ")")
    TestUtil.Assert([-922337203685477.5808 1] <= -[922337203685477.5807 1], [-922337203685477.5808 1] & " <= " & -[922337203685477.5807 1])
    TestUtil.Assert(Not([-922337203685477.5808 1] >= -[922337203685477.5807 1]), "Not(" & [-922337203685477.5808 1] & " >= " & -[922337203685477.5807 1] & ")")

    TestUtil.AssertEqual([-922337203685477.5808 1] + [922337203685477.5807 1], -0.0001@, [-922337203685477.5808 1] & " + " & [922337203685477.5807 1])

    ' It is not possible to negate -922337203685477.5808, because 922337203685477.5808 is not representable (max is 922337203685477.5807)
    verify_NegationOverflow([-922337203685477.5808 1])

    ' Different overflows
    verify_AddOverflow([922337203685477.5807 1], 0.0001@)
    verify_AddOverflow([-922337203685477.5808 1], -0.0001@)

    verify_SubOverflow([922337203685477.5807 1], -0.0001@)
    verify_SubOverflow([-922337203685477.5808 1], 0.0001@)

    verify_MulOverflow([922337203685477.5807 1], 1.1@)
    verify_MulOverflow([-922337203685477.5808 1], 1.1@)

    verify_DivOverflow([922337203685477.5807 1], 0.1@)
    verify_DivOverflow([-922337203685477.5808 1], 0.1@)

    x = 0.1@ ' Must round to 0 in Not, and complement is -1
    TestUtil.AssertEqual(Not x, -1, "Not " & x)
    x = 0.6@ ' Must round to 1 in Not, and complement is -2
    TestUtil.AssertEqual(Not x, -2, "Not " & x)
    ' TODO: fix compile-time constant operations: rounding is wrong
    TestUtil.AssertEqual(Not 0.1@, -1, "Not 0.1@")
    ' TestUtil.AssertEqual(Not 0.6@, -2, "Not 0.6@") ' Fails, gives -1

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testCurrency", Err, Error$, Erl)
End Sub

Sub verify_DivideByZero(x As Currency, y As Currency)
    On Error GoTo errorHandler

    x = x / y
    TestUtil.Assert(False, "verify_DivideByZero", x & " / " & y & ": Division by zero expected")

    Exit Sub
errorHandler:
    TestUtil.AssertEqual(Err, 11, x & " / " & y & " gave " & Error$)
End Sub

Sub verify_IDivideByZero(x As Currency, y As Currency)
    On Error GoTo errorHandler

    x = x \ y
    TestUtil.Assert(False, "verify_IDivideByZero", x & " \ " & y & ": Division by zero expected")

    Exit Sub
errorHandler:
    TestUtil.AssertEqual(Err, 11, x & " \ " & y & " gave " & Error$)
End Sub

Sub verify_IModByZero(x As Currency, y As Currency)
    On Error GoTo errorHandler

    x = x Mod y
    TestUtil.Assert(False, "verify_IModByZero", x & " Mod " & y & ": Division by zero expected")

    Exit Sub
errorHandler:
    TestUtil.AssertEqual(Err, 11, x & " Mod " & y & " gave " & Error$)
End Sub

Sub verify_NegationOverflow(x As Currency)
    On Error GoTo errorHandler

    x = -x
    TestUtil.Assert(False, "verify_NegationOverflow", "-" & x & ": Overflow expected")

    Exit Sub
errorHandler:
    TestUtil.AssertEqual(Err, 6, "-" & x & " gave " & Error$)
End Sub

Sub verify_AddOverflow(x As Currency, y As Currency)
    On Error GoTo errorHandler

    x = x + y
    TestUtil.Assert(False, "verify_AddOverflow", x & " + " & y & ": Overflow expected")

    Exit Sub
errorHandler:
    TestUtil.AssertEqual(Err, 6, x & " + " & y & " gave " & Error$)
End Sub

Sub verify_SubOverflow(x As Currency, y As Currency)
    On Error GoTo errorHandler

    x = x - y
    TestUtil.Assert(False, "verify_SubOverflow", x & " - " & y & ": Overflow expected")

    Exit Sub
errorHandler:
    TestUtil.AssertEqual(Err, 6, x & " - " & y & " gave " & Error$)
End Sub

Sub verify_MulOverflow(x As Currency, y As Currency)
    On Error GoTo errorHandler

    x = x * y
    TestUtil.Assert(False, "verify_MulOverflow", x & " * " & y & ": Overflow expected")

    Exit Sub
errorHandler:
    TestUtil.AssertEqual(Err, 6, x & " * " & y & " gave " & Error$)
End Sub

Sub verify_DivOverflow(x As Currency, y As Currency)
    On Error GoTo errorHandler

    x = x / y
    TestUtil.Assert(False, "verify_DivOverflow", x & " / " & y & ": Overflow expected")

    Exit Sub
errorHandler:
    TestUtil.AssertEqual(Err, 6, x & " / " & y & " gave " & Error$)
End Sub
