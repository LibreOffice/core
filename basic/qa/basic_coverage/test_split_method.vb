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
    result = verify_testSplit()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function

Function verify_testSplit() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    ' SPLIT
    TestLog_ASSERT Split( "Hello world" )(1), "world", "Split( ""Hello world"" )(1)"

    ' tdf#123025 - split function sets the datatype of the array to empty,
    ' preventing any subsequent assignments of values to the array and to the elements itself.
    Dim arr(1) As String
    arr = Split("a/b", "/")
    TestLog_ASSERT arr(0), "a", "Split(""a/b"", ""/"")(0)"
    TestLog_ASSERT arr(1), "b", "Split(""a/b"", ""/"")(1)"
    ReDim Preserve arr(1)
    TestLog_ASSERT arr(0), "a", "ReDim Preserve arr(1)(0)"
    TestLog_ASSERT arr(1), "b", "ReDim Preserve arr(1)(1)"
    ReDim arr(1)
    TestLog_ASSERT arr(0), "", "ReDim arr(1)(0)"
    TestLog_ASSERT arr(1), "", "ReDim arr(1)(1)"

    arr(0) = "a"
    arr(1) = "b"
    TestLog_ASSERT arr(0), "a", "arr(0)"
    TestLog_ASSERT arr(1), "b", "arr(1)"
    ReDim Preserve arr(1)
    TestLog_ASSERT arr(0), "a", "ReDim Preserve arr(1)(0) after assignment"
    TestLog_ASSERT arr(1), "b", "ReDim Preserve arr(1)(1) after assignment"

    ' tdf#144924 - allow the assignment of different data types to the individual elements
    Dim splitArr
    splitArr = Split("a/b&&c/d", "&&")
    Dim i As Integer
    For i = 0 To UBound(splitArr)
        ' Without the fix in place, this assignment would have failed
        splitArr(i) = Split(splitArr(i), "/")
        ' Without the fix in place, this test would have failed with:
        ' - Expected: 8200 (8192 for Array and 8 for String)
        ' - Actual  : 8    (8 for String)
        TestLog_ASSERT VarType(splitArr(i)), 8200, "VarType(splitArr(i))"
    Next

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testSplit = result

End Function

Sub TestLog_ASSERT(actual As Variant, expected As Variant, testName As String)
    If expected = actual Then
        passCount = passCount + 1
    Else
        result = result & Chr$(10) & "Failed: " & testName & " returned " & actual & ", expected " & expected
        failCount = failCount + 1
    End If
End Sub
