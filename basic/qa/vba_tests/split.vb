'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testSplit
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testSplit
    On Error GoTo errorHandler

    ' SPLIT
    TestUtil.AssertEqual(Split( "Hello world" )(1), "world", "Split( ""Hello world"" )(1)")

    ' tdf#123025 - split function sets the datatype of the array to empty,
    ' preventing any subsequent assignments of values to the array and to the elements itself.
    Dim arr(1) As String
    arr = Split("a/b", "/")
    TestUtil.AssertEqual(arr(0), "a", "Split(""a/b"", ""/"")(0)")
    TestUtil.AssertEqual(arr(1), "b", "Split(""a/b"", ""/"")(1)")
    ReDim Preserve arr(1)
    TestUtil.AssertEqual(arr(0), "a", "ReDim Preserve arr(1)(0)")
    TestUtil.AssertEqual(arr(1), "b", "ReDim Preserve arr(1)(1)")
    ReDim arr(1)
    TestUtil.AssertEqual(arr(0), "", "ReDim arr(1)(0)")
    TestUtil.AssertEqual(arr(1), "", "ReDim arr(1)(1)")

    arr(0) = "a"
    arr(1) = "b"
    TestUtil.AssertEqual(arr(0), "a", "arr(0)")
    TestUtil.AssertEqual(arr(1), "b", "arr(1)")
    ReDim Preserve arr(1)
    TestUtil.AssertEqual(arr(0), "a", "ReDim Preserve arr(1)(0) after assignment")
    TestUtil.AssertEqual(arr(1), "b", "ReDim Preserve arr(1)(1) after assignment")

    ' tdf#144924 - using VBASupport 1, the split function returns an array of substrings, hence no
    ' assignment of different data types to the individual elements is possible
    Dim splitArr
    splitArr = Split("a/b&&c/d", "&&")
    ' Without the fix in place, this test would have failed with:
    ' - Expected: 8    (8 for String)
    ' - Actual  : 8200 (8192 for Array and 8 for String)
    TestUtil.AssertEqual(VarType(splitArr(0)), 8, "VarType(splitArr(0))")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testSplit", Err, Error$, Erl)
End Sub
