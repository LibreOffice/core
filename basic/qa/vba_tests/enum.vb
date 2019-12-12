'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Enum CountDown ' Values are rendered to Long
    FIVE = 4.01
    FOUR = -4.01
    THREE = 5
    TWO = -.3E1
    ONE = 300.0E-2
    LIFT_OFF = 7
End Enum ' CountDown

Sub Main : doUnitTest : MsgBox result : End Sub 'only for DEV/TEST

Function doUnitTest()
    ''' test_vba.cxx main entry point '''
    verify_testENUM
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = result
    Else
        doUnitTest = "OK"
    End If
End Function

Sub verify_testENUM()

    passCount = 0
    failCount = 0

    result = "Test Results" & vbNewLine & "============" & vbNewLine

    Dim testName As String
    testName = "Test Enum Statement"
try:
    On Error Goto catch

    With CountDown

        TestLog_ASSERT .ONE = 3, "CountDown.ONE equals " & Str(.ONE)
        TestLog_ASSERT .TWO = -3, "CountDown.TWO equals " & Str(.TWO)

        TestLog_ASSERT TypeName(.FOUR) = "Long", "CountDown.FOUR type is: " & TypeName(.FOUR)

        Dim sum As Double
        sum = .FIVE + .FOUR + .THREE + .TWO + .ONE + .LIFT_OFF
        TestLog_Assert sum = 12, "SUM of CountDown values is: " & Str(sum)

    End With ' CountDown

finally:
    result = result & vbNewLine & "Tests passed: " & passCount & vbNewLine & "Tests failed: " & failCount & vbNewLine
    Exit Sub

catch:
    TestLog_ASSERT (False), testName & ": hit error handler"
    Resume finally
End Sub

Sub TestLog_ASSERT(assertion As Boolean, Optional testId As String, Optional testComment As String)

    If assertion = True Then
        passCount = passCount + 1
    Else
        Dim testMsg As String
        If Not IsMissing(testId) Then
            testMsg = testMsg + " : " + testId
        End If
        If Not IsMissing(testComment) And Not (testComment = "") Then
            testMsg = testMsg + " (" + testComment + ")"
        End If

        result = result & vbNewLine & " Failed: " & testMsg
        failCount = failCount + 1
    End If

End Sub
