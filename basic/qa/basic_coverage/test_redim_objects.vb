' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Type testType
    iNr    As Integer
    sType  As String
    aValue As Variant
    oColor As Object
End Type

Function doUnitTest as Integer

    doUnitTest = 0

    ' tdf#136755 - ReDim did not work on an array of objects
    Dim aPropertyValues(1) As New com.sun.star.beans.PropertyValue
    If (UBound(aPropertyValues) <> 1) Then Exit Function
    ReDim aPropertyValues(5) As com.sun.star.beans.PropertyValue
    If (UBound(aPropertyValues) <> 5) Then Exit Function

    ' tdf#124008 - ReDim did not work on an array of individual declared types
    Dim aType(1) As testType
    If (UBound(aType) <> 1) Then Exit Function
    ReDim aType(5) As testType
    If (UBound(aType) <> 5) Then Exit Function

    doUnitTest = 1

End Function
