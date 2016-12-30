'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Type address
    Name1 As String
    City As String
End Type

Function doUnitTest as Integer
    ' CREATEOBJECT
    If ( IsObject( CreateObject("address") ) = False ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
