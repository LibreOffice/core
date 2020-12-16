'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Option Explicit
#include _test_header.vb
#include _test_asserts.vb

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
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = result
    Else
        doUnitTest = "OK"
    End If
End Function


Sub ENUM_TestCases()

    passCount = 0
    failCount = 0

    result = "Test Results" & vbNewLine & "============" & vbNewLine

try:
    On Error Goto catch

    With CountDown

a:      TestLog_ASSERT .ONE = 3, "case a", "CountDown.ONE equals " & Str(.ONE)

b:      TestLog_ASSERT .TWO = -3, "case b", "CountDown.TWO equals " & Str(.TWO)

c:      TestLog_ASSERT TypeName(.FOUR) = "Long", "case c", "CountDown.FOUR type is: " & TypeName(.FOUR)

d:      Dim sum As Double
        sum = .FIVE + .FOUR + .THREE + .TWO + .ONE + .LIFT_OFF
        TestLog_Assert sum = 12, "case d", "SUM of CountDown values is: " & Str(sum)

    End With ' CountDown

finally:
    result = result & vbNewLine & "Tests passed: " & passCount & vbNewLine & "Tests failed: " & failCount & vbNewLine
    Exit Sub

catch:
    TestLog_ASSERT (False), "ERROR", "#"& Str(Err.Number) &" in 'ENUM_TestCases' at line"& Str(Erl) &" - "& Error$
    Resume Next
End Sub


'Sub DEV_TEST : doUnitTest : MsgBox result : End Sub
