'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Compatible

Function doUnitTest as Integer
  ' PROPERTY GET/LET
  aString = "Office"
  If ( aString <> "LibreOffice") Then
    doUnitTest = 0 ' Ko
  Else
    doUnitTest = 1 ' Ok
  End If
End Function

Dim _pn As String
Property Get aString As String
    aString = _pn
End Property
Property Let aString(value As String)
    _pn = "Libre"& value
End Property
