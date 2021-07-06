' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
    ' CreateUnoStruct
    Dim oStruct
    oStruct = CreateUnoStruct( "com.sun.star.beans.Property" )
    doUnitTest = "OK"
End Function
