Option VBASupport 1
Option Explicit

'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'
' Test built-in compatibility versions of methods whose absence
' is really felt in VBA, and large numbers of macros import from
' the system.
'

Private Declare Function QueryPerformanceCounter Lib "kernel32" (ByRef lpPerformanceCount As Currency) As Long
Private Declare Function QueryPerformanceFrequency Lib "kernel32" (ByRef lpFrequency As Currency) As Long

Function doUnitTest() As String
    verify_win32compat
    doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_win32compat()
    TestUtilModule.TestInit()

    Dim freq As Currency
    Dim count_a As Currency
    Dim count_b As Currency
    Dim success As Long

    On Error GoTo errorHandler

    success = QueryPerformanceFrequency(freq)
    TestUtilModule.AssertTrue(success <> 0, "fetching perf. frequency")
    TestUtilModule.AssertTrue(freq > 0, "perf. frequency is incorrect " & freq)

    success = QueryPerformanceCounter(count_a)
    TestUtilModule.AssertTrue(success <> 0, "fetching performance count")

    success = QueryPerformanceCounter(count_b)
    TestUtilModule.AssertTrue(success <> 0, "fetching performance count")
    TestUtilModule.AssertTrue(count_a < count_b, "count mismatch " & count_a & " is > " & count_b)
    Exit Sub

errorHandler:
    TestUtilModule.AssertTrue(False, "hit error handler - " & Err & ": " & Error$ & " (line : " & Erl & ")")

End Sub
