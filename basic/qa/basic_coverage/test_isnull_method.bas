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
    verify_testIsNull
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testIsNull()
    On Error GoTo errorHandler

    Dim aVariant as Variant
    TestUtil.Assert(Not IsNull(aVariant), "Not IsNull(aVariant)")

    ' tdf#146112 In Basic IsNull returns True for empty objects
    ' The type name is "Object" in this case
    Dim aEmptyObj As object
    TestUtil.Assert(IsNull(aEmptyObj), "IsNull(aEmptyObj)")
    TestUtil.AssertEqual(TypeName(aEmptyObj), "Object", "TypeName(aEmptyObj)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testDateAdd", Err, Error$, Erl)
End Sub
