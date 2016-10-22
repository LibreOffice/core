'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    dim aVector as Variant
    ' ARRAY
    aVector = Array( Date(), Time(), "Hello", -3.14)
    If (aVector(2) <> "Hello") Then
        doUnitTest = 0
    ' LBOUND
    Else If ( LBound( aVector() ) <> 0 ) Then
        doUnitTest = 0
    ' TYPELEN
    Else If ( TypeLen(aVector(2)) <> 5 ) Then
        doUnitTest = 0
    ' TYPENAME
    Else If ( TypeName(aVector(2)) <> "String" ) Then
        doUnitTest = 0
    ' UBOUND
    Else If ( UBound( aVector() ) <> 3 ) Then
        doUnitTest = 0
    ' VARTYPE
    Else If ( VarType(aVector(2)) <> 8 ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
    End If
    End If
    End If
    End If
    End If
End Function
