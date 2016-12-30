' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    ' CreateUnoValue
    Dim oUnoValue as Variant
    Dim aValue as Variant
    aValue = Array ( 1, 1 )
    oUnoValue = CreateUnoValue( "[]byte", aValue )
    doUnitTest = 1
End Function
