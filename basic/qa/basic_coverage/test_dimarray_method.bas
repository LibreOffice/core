' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
    Dim aValue as variant
    aValue = DimArray( 1, 2, 4 )
    aValue( 1, 2, 4 ) = 3
    ' DIMARRAY
    If ( aValue( 1, 2, 4 ) <> 3 ) Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    End If
End Function
