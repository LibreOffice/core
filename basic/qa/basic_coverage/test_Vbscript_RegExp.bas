' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testVbscript_RegExp
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testVbscript_RegExp()
    On Error GoTo errorHandler

    If GetGuiType() <> 1 Then ' Only testing on Windows
        TestUtil.Assert(True) ' Mark test as succeeded
        Exit Sub
    End If

    Dim txt, regex, matches, item1, item2

    txt = "This is LibreOffice for you"

    regex = CreateObject("VBScript.RegExp")
    regex.Pattern = "(Li.+)(Of\w+)"

    matches = regex.Execute(txt)

    ' Test that accessing twice an indexed property (here: Item) of a COM object works
    If matches.Count > 0 Then
        item1 = matches.Item(0).SubMatches.Item(0)
        ' Without the fix, the following line errored out:
        ' Type: com.sun.star.lang.WrappedTargetRuntimeException
        ' Message: [automation bridge] unexpected exception in IUnknownWrapper::getValue
        item2 = matches.Item(0).SubMatches.Item(1)
    End If

    TestUtil.AssertEqual(item1, "Libre", "item1")
    TestUtil.AssertEqual(item2, "Office", "item2")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testVbscript_RegExp", Err, Error$, Erl)
End Sub
