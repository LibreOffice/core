'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
  ''' Return 'text' OR 'date' variable '''

  If (TypeName(Time$)<>"String" Or Vartype(Time())<>V_DATE) Then
    doUnitTest = "FAIL" ' not successful
  Else
    doUnitTest = "OK" ' Ok
  End If
End Function

Sub DEV_TST : MsgBox doUnitTest : End Sub