'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
    TestUtil.TestInit
    test_with
    doUnitTest = TestUtil.GetResult()
End Function

Sub DEV_TST : MsgBox doUnitTesT : End Sub

Type foo
    n As Integer
    s As String
End Type

Dim get_foo_count As Integer

Function get_foo As foo
    get_foo_count = get_foo_count + 1
    get_foo = New foo
End Function

Sub test_with
    On Error GoTo errorHandler

    Dim fields As String
    With get_foo()
        .n = 5
        .s = "bar"
        fields = "n = " & .n & " s = " & .s
    End With
    ' get_foo must be called only once; before the fix, it failed with
    '  Number of calls to get_foo returned 4, expected 1
    TestUtil.AssertEqual(get_foo_count, 1, "Number of calls to get_foo")
    ' Before the fix, each use of . resulted in creation of a new 'foo' object,
    ' and previous assignments didn't reflect in the result; it failed with
    '  Field values returned n = 0 s = , expected n = 5 s = bar
    TestUtil.AssertEqual(fields, "n = 5 s = bar", "Field values")

    ' Make sure that With works with the original object, modifies it, and does not destroy
    Dim foo_var As New foo
    With foo_var
        .n = 6
        .s = "baz"
    End With
    fields = "n = " & foo_var.n & " s = " & foo_var.s
    TestUtil.AssertEqual(fields, "n = 6 s = baz", "Field values of foo_var")

    ' tdf#162935: Test an UNO struct - it used to copy into the With variable, not used by ref
    Dim uno_struct As New com.sun.star.table.CellRangeAddress
    With uno_struct
        .Sheet = 1
        .StartColumn = 2
        .StartRow = 3
        .EndColumn = 4
        .EndRow = 5
    End With
    TestUtil.AssertEqual(uno_struct.Sheet, 1, "uno_struct.Sheet")
    TestUtil.AssertEqual(uno_struct.StartColumn, 2, "uno_struct.StartColumn")
    TestUtil.AssertEqual(uno_struct.StartRow, 3, "uno_struct.StartRow")
    TestUtil.AssertEqual(uno_struct.EndColumn, 4, "uno_struct.EndColumn")
    TestUtil.AssertEqual(uno_struct.EndRow, 5, "uno_struct.EndRow")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("test_with", Err, Error$, Erl)
End Sub
