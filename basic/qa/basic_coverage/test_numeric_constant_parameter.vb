'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

' assigns a numeric constant (integer) to a parameter of type variant
Function assignInteger( numericConstant ) As String
    numericConstant = 1
    assignInteger = TypeName( numericConstant )
End Function

' assigns a numeric constant (long) to a parameter of type variant
Function assignLong( numericConstant ) As String
    numericConstant = 32768
    assignLong = TypeName( numericConstant )
End Function

Function doUnitTest() As Integer
    ' tdf#133913 - check if numeric constants are converted correctly to
    ' their respective types, if they are passed as arguments to a function
    ' with variant parameter types.
    On Error GoTo errorHandler
    If (assignInteger( 1 ) = "Integer" And assignLong( 1 ) = "Long") Then
        doUnitTest = 1
    Else
        doUnitTest = 0
    End If
    Exit Function
errorHandler:
    doUnitTest = 0
End Function