'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_VariantArgUsedAsObj
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_VariantArgUsedAsObj
    On Error GoTo errorHandler

    Dim aResult
    ' Without the fix, the following line would fail, triggering errorHandler reporting
    ' "91: Object variable not set."
    aResult = aFuncWithVarArg(0)
    TestUtil.AssertEqualStrict(aResult, "Integer", "aResult")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_VariantArgUsedAsObj", Err, Error$, Erl)
End Sub

Function aFuncWithVarArg(arg)
    ' The 'arg' is implicitly Variant; and its following use as object (after a check),
    ' i.e. accessing its method using dot notation, must not change the declaration type
    If IsObject(arg) Then
        arg.some_func()
    End If
    aFuncWithVarArg = TypeName(arg)
End Function
