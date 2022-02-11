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

    If (GetGUIType() <> 1) Then Exit Sub ' Only test on Windows

    ' Wihtout the fix for tdf#147364 in place, the call to PathFindExtensionA would crash with
    '   *** Exception 0xc0000005 occurred ***
    TestUtil.AssertEqual(PathFindExtensionA("filename.ext"), ".ext", "PathFindExtensionA(""filename.ext"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testPathFindExtensionA", Err, Error$, Erl)
End Sub
