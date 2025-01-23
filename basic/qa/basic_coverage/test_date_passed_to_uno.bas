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
    verify_DatePassedToUno
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_DatePassedToUno
    On Error GoTo errorHandler

    ' Given a date variable, and passing it to an UNO interface method (i.e., going through
    ' the Basic-to-UNO conversion), in the absence of VBASupport mode, the date value must
    ' be converted to a number, not to a struct / empty interface
    Dim aDate as Date
    aDate = #2025-01-23T12:00#

    Dim aUnoObject as Object
    aUnoObject = CreateUnoListener("XTypeConverter_", "com.sun.star.script.XTypeConverter")

    Dim aConvResult
    ' Without the fix, the following line would fail, triggering errorHandler reporting
    ' "91: Object variable not set."
    aConvResult = aUnoObject.convertToSimpleType(aDate, com.sun.star.uno.TypeClass.STRING)
    ' If 'Option VBASupport 1' were used, the following line would fail with "Wrong input
    ' type: Date", because the value would be marshalled using oleautomation::Date struct
    TestUtil.AssertEqualStrict(aConvResult, "45680.5", "aConvResult")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_DatePassedToUno", Err, Error$, Erl)
End Sub

Function XTypeConverter_convertToSimpleType(aFrom, aDestinationType)
    If IsNumeric(aFrom) Then
        XTypeConverter_convertToSimpleType = CStr(aFrom)
    Else
        XTypeConverter_convertToSimpleType = "Wrong input type: " & TypeName(aFrom)
    End If
End Function
