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

Option VBASupport 1
Option Explicit

Private Declare Function QueryPerformanceCounter Lib "kernel32" (ByRef lpPerformanceCount As Currency) As Long
Private Declare Function QueryPerformanceFrequency Lib "kernel32" (ByRef lpFrequency As Currency) As Long

Function doUnitTest() As String
    TestUtil.TestInit()
    verify_win32compat
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_win32compat()
    Dim freq As Currency
    Dim count_a As Currency
    Dim count_b As Currency
    Dim success As Long

    On Error GoTo errorHandler

    success = QueryPerformanceFrequency(freq)
    TestUtil.Assert(success <> 0, "QueryPerformanceFrequency")
    TestUtil.Assert(freq > 0, "QueryPerformanceFrequency", "perf. frequency is incorrect " & freq)

    success = QueryPerformanceCounter(count_a)
    TestUtil.Assert(success <> 0, "QueryPerformanceCounter(count_a)")

    success = QueryPerformanceCounter(count_b)
    TestUtil.Assert(success <> 0, "QueryPerformanceCounter(count_b)")
    TestUtil.Assert(count_a < count_b, "count mismatch " & count_a & " is > " & count_b)

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_win32compat", Err, Error$, Erl)
End Sub
