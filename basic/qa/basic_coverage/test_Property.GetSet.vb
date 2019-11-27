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
  Set anObject = CreateUnoService( "com.sun.star.frame.Desktop" )
  If ( anObject.SupportedServiceNames(0) <> "com.sun.star.frame.Frame") Then
    doUnitTest = 0 ' not Ok
  Else
    doUnitTest = 1 ' Ok
  End If
End Function

Sub DEV_TST : MsgBox doUnitTesT : End Sub

Property Get anObject As Object
  Set anObject = _obj
End Property

Private _obj As Object

Property Set anObject(value As Object)
  Set _obj = value.CurrentFrame
End Property
