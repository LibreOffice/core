'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
    result = verify_testCStr()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function

Function verify_testCStr() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    ' CSTR
    TestLog_ASSERT CStr(100), "100", "CStr(100)"

    ' tdf#143575 - round string to their nearest double representation
    ' Without the fix in place, this test would have failed with:
    ' - Expected: 691.2
    ' - Actual  : 691.2000000000001
    TestLog_ASSERT CStr(691.2), "691.2", "CStr(691.2)"
    ' Without the fix in place, this test would have failed with:
    ' - Expected: 691.2
    ' - Actual  : 691.1999999999999
    TestLog_ASSERT CStr(123.4 + 567.8), "691.2", "CStr(123.4 + 567.8)"

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testCStr = result
End Function

Sub TestLog_ASSERT(actual As Variant, expected As Integer, testName As String)
    If expected = actual Then
        passCount = passCount + 1
    Else
        result = result & Chr$(10) & " Failed: " & testName & " returned " & actual & ", expected " & expected
        failCount = failCount + 1
    End If
End Sub
