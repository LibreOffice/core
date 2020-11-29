' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Option Explicit

Type testType
    iNr    As Integer
    sType  As String
    aValue As Variant
    oColor As Object
End Type

Function doUnitTest() As String
     TestUtil.TestInit
     verify_testReDimObjects
     doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testReDimObjects()
    On Error GoTo errorHandler

    ' tdf#136755 - ReDim did not work on an array of objects
    Dim aPropertyValues(1) As New com.sun.star.beans.PropertyValue
    TestUtil.AssertEqual(UBound(aPropertyValues), 1, "UBound(aPropertyValues)")
    ReDim aPropertyValues(5) As com.sun.star.beans.PropertyValue
    TestUtil.AssertEqual(UBound(aPropertyValues), 5, "UBound(aPropertyValues)")

    ' tdf#124008 - ReDim did not work on an array of individual declared types
    Dim aType(1) As testType
    TestUtil.AssertEqual(UBound(aType), 1, "UBound(aType)")
    ReDim aType(5) As testType
    TestUtil.AssertEqual(UBound(aType), 5, "UBound(aType)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testReDimObjects", Err, Error$, Erl)
End Sub
