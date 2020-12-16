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
    Call ENUM_TestCases
    doUnitTest = TestUtilModule.GetResult()
End Function

Sub ENUM_TestCases()
    TestUtilModule.TestInit
try:
    On Error Goto catch

    With CountDown

a:      TestUtilModule.AssertTrue(.ONE = 3, "case a", "CountDown.ONE equals " & Str(.ONE))

b:      TestUtilModule.AssertTrue(.TWO = -3, "case b", "CountDown.TWO equals " & Str(.TWO))

c:      TestUtilModule.AssertTrue(TypeName(.FOUR) = "Long", "case c", "CountDown.FOUR type is: " & TypeName(.FOUR))

d:      Dim sum As Double
        sum = .FIVE + .FOUR + .THREE + .TWO + .ONE + .LIFT_OFF
        TestUtilModule.AssertTrue(sum = 12, "case d", "SUM of CountDown values is: " & Str(sum))

    End With

finally:
    TestUtilModule.TestEnd
    Exit Sub

catch:
    TestUtilModule.AssertTrue(False, "ERROR", "#"& Str(Err.Number) &" in 'ENUM_TestCases' at line"& Str(Erl) &" - "& Error$)
    Resume Next
End Sub
