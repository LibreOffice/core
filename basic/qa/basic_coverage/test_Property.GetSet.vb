'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Compatible

Function doUnitTest as Integer
  ' PROPERTY GET/SET for classes or UNO services

  Set unoSetter = CreateUnoService( "com.sun.star.frame.Desktop" )
  ' OR  unoSetter = CreateUnoService( "com.sun.star.frame.Desktop" )
  Set objSetter = New Collection ' OR objLetter = New Collection
  If ( Not unoGetter.supportsService("com.sun.star.frame.Frame") _
        Or objGetter.Count <> 3 ) Then
    doUnitTest = 0 ' not Ok
  Else
    doUnitTest = 1 ' Ok
  End If
End Function

Sub DEV_TST : MsgBox doUnitTesT : End Sub

Property Get unoGetter As Object
  Set unoGetter = UNObj
End Property

Dim UNObj As Object

Property Set unoSetter(value As Object)
  Set UNObj = value.CurrentFrame
End Property

'Property Let unoLetter(value As Object)
  'Set _uno = value.CurrentFrame
'End Property

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
End Property

'Property Let objLetter(value As Object)
  'Set _obj = value
  'With _obj
    '.add "branches"
    '.add "leaves"
  'End With
'End Property
