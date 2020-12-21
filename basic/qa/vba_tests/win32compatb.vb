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
' This module tests different signatures for the same methods.
'

Option VBASupport 1
Option Explicit

Private Type LARGE_INTEGER
    lowpart As Long
    highpart As Long
End Type

Private Declare Function QueryPerformanceCounter Lib "kernel32" (lpPerformanceCount As LARGE_INTEGER) As Long
Private Declare Function QueryPerformanceFrequency Lib "kernel32" (lpFrequency As LARGE_INTEGER) As Long

Function doUnitTest() As String
    TestUtil.TestInit
    verify_win32compatb
    doUnitTest = TestUtil.GetResult()
End Function

Function convertLarge(scratch As LARGE_INTEGER) As Double
    Dim ret As Double
    ret = scratch.highpart
    ret = ret * 65536 * 65536
    ret = ret + scratch.lowpart
    convertLarge = ret
End Function

Sub verify_win32compatb()
    Dim scratch as LARGE_INTEGER
    Dim freq As Double
    Dim count_a As Double
    Dim count_b As Double
    Dim success As Long

    On Error GoTo errorHandler

    success = QueryPerformanceFrequency(scratch)
    TestUtil.Assert(success <> 0, "QueryPerformanceFrequency")
    freq = convertLarge(scratch)
    TestUtil.Assert(freq > 0, "QueryPerformanceFrequency", "perf. frequency is incorrect " & freq)

    success = QueryPerformanceCounter(scratch)
    TestUtil.Assert(success <> 0, "QueryPerformanceCounter")
    count_a = convertLarge(scratch)

'    success = QueryPerformanceCounter(scratch)
'    TestUtil.Assert(success <> 0, "fetching performance count")
'    count_b = convertLarge(scratch)
'    TestUtil.Assert(count_a < count_b, "count mismatch " & count_a & " is > " & count_b)

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_win32compatb", Err, Error$, Erl)
End Sub
