Option VBASupport 1
Option Explicit

'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'
'
' Test built-in compatibility versions of methods whose absence
' is really felt in VBA, and large numbers of macros import from
' the system.
'

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Private Declare Function QueryPerformanceCounter Lib "kernel32" (ByRef lpPerformanceCount As Currency) As Long
Private Declare Function QueryPerformanceFrequency Lib "kernel32" (ByRef lpFrequency As Currency) As Long

' FIXME: all this cut/paste should be factored out !

Function doUnitTest() As String
    result = verify_win32compat()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = result
    Else
        doUnitTest = "OK"
    End If
End Function


Function verify_win32compat() as String
    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "================" & Chr$(10)

    Dim freq As Currency
    Dim count_a As Currency
    Dim count_b As Currency
    Dim success As Long

    On Error GoTo errorHandler

    success = QueryPerformanceFrequency(freq)
    TestLog_ASSERT success <> 0, "fetching perf. frequency"
    TestLog_ASSERT freq > 0, "perf. frequency is incorrect " & freq

    success = QueryPerformanceCounter(count_a)
    TestLog_ASSERT success <> 0, "fetching performance count"

    success = QueryPerformanceCounter(count_b)
    TestLog_ASSERT success <> 0, "fetching performance count"
    TestLog_ASSERT count_a < count_b, "count mismatch " & count_a & " is > " & count_b

    verify_win32compat = "OK"
    Exit Function

errorHandler:
    TestLog_ASSERT (False), "hit error handler - " & Err & ": " & Error$ & " (line : " & Erl & ")"
    verify_win32compat = result

End Function

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

        result = result & Chr$(10) & " Failed: " & testMsg
        failCount = failCount + 1
    End If

End Sub
