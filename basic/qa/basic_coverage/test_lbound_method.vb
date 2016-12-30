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
    aVector = Array( "Hello", -3.14)
    ' LBOUND
    If ( LBound( aVector() ) <> 0 ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
