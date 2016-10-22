'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    dim aVector as Variant
    aVector = Array( Date(), Time(), "Hello", -3.14)
    ' CHOOSE
    If (Choose(2, 1, 100, 3) <> 100) Then
        doUnitTest = 0
    ' FALSE TRUE
    Else If (False = True) Then
        doUnitTest = 0
    ' IIF
    Else If ( IIF(True, 10, 12) <> 10 ) Then
        doUnitTest = 0
    ' ISARRAY
    Else If ( IsArray( aVector ) = False ) Then
        doUnitTest = 0
    ' ISDATE
    Else If ( IsDate( aVector(1) ) = False ) Then
        doUnitTest = 0
    ' ISEMPTY
    Else If ( IsEmpty( aVector(2) ) ) Then
        doUnitTest = 0
    ' ISERROR
    ' ISMISSING
    ' ISNULL
    Else If ( IsNull( aVector ) ) Then
        doUnitTest = 0
    ' ISNUMERIC
    Else If ( IsNumeric( aVector(3) ) = False ) Then
        doUnitTest = 0
    ' IsUnoStruc
    ' SWITCH
    Else If ( Switch( False, aVector(0),_
                      True,  aVector(1),_
                      False, aVector(2),_
                      True,  aVector(3)  ) <> aVector(1) ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    End If
End Function
