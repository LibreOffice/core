'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit
Option Compatible

Function doUnitTest as String
  ' PROPERTY GET/SET for classes or UNO services

  Set objSetter = New Collection ' OR objLetter = New Collection
  If ( objGetter.Count <> 3 ) Then
    doUnitTest = "FAIL" ' not Ok
  Else
    doUnitTest = "OK" ' Ok
  End If
End Function

Sub DEV_TST : MsgBox doUnitTesT : End Sub

Property Get objGetter As Object
  _obj.add "roots"
  Set objGetter = _obj
End Property

Private _obj As Object

Property Set objSetter(value As Object)
  Set _obj = value
  With _obj
    .add "branches"
    .add "leaves"
  End With
  Exit Property ' i#109051
End Property
