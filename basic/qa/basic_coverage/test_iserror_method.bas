'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Type MyType
    tName as String
End Type

Function doUnitTest as String
    dim aVariant as MyType
    aVariant.tName = "A string"
    ' ISERROR
    If ( IsError( aVariant ) ) Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    End If
End Function
