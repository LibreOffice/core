'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Option Explicit

Enum CountDown ' Values get ROUNDED to Int32
    FIVE = 4.11
    FOUR = -4.25
    THREE = 5
    TWO = -.315E1
    ONE = 286.0E-2 ' equals 3
    LIFT_OFF = 7
End Enum ' CountDown

Function doUnitTest()
    ''' test_vba.cxx main entry point '''
    TestUtil.TestInit
    Call ENUM_TestCases
    doUnitTest = TestUtil.GetResult()
End Function

Sub ENUM_TestCases()
try:
    On Error Goto catch

    With CountDown

a:      TestUtil.AssertEqual(.ONE, 3, ".ONE")

b:      TestUtil.AssertEqual(.TWO, -3, ".TWO")

c:      TestUtil.AssertEqual(TypeName(.FOUR), "Long", "TypeName(.FOUR)")

d:      Dim sum As Double
        sum = .FIVE + .FOUR + .THREE + .TWO + .ONE + .LIFT_OFF
        TestUtil.AssertEqual(sum, 12, "sum")

    End With

finally:
    Exit Sub

catch:
    TestUtil.ReportErrorHandler("ENUM_TestCases", Err, Error$, Erl)
    Resume Next
End Sub
