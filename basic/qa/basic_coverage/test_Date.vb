'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
  ''' Return 'text' OR 'date' variable '''

  If ( TypeName(Date$)<>"String" Or Vartype(Date())<>V_DATE) Then
    doUnitTest = 0 ' not successful
  Else
    doUnitTest = 1 ' Ok
  End If
End Function

Sub DEV_TST : MsgBox doUnitTesT : End Sub