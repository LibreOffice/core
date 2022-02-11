'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

' We link to shlwapi in many places, so safe to rely on it here
Declare Function PathFindExtensionA Lib "Shlwapi" (ByVal pszPath As String) As String

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testPathFindExtensionA
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testPathFindExtensionA()
    On Error GoTo errorHandler

    ' Only test on Windows
    If (GetGUIType() <> 1) Then
        TestUtil.Assert(True) ' The test passed
        Exit Sub
    End If

    ' Without the fix for tdf#147364 in place, the next call would crash in 64-bit version with
    '   *** Exception 0xc0000005 occurred ***
    TestUtil.AssertEqual(PathFindExtensionA("filename.ext"), ".ext", "PathFindExtensionA(""filename.ext"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testPathFindExtensionA", Err, Error$, Erl)
End Sub
