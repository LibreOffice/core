'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit
Type address
    Name1 As String
    City As String
End Type

Function doUnitTest as String
    ' CREATEOBJECT
    If ( IsObject( CreateObject("address") ) = False ) Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    End If
End Function
