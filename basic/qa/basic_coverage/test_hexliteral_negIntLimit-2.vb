'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'


Function doUnitTest as Integer
  Dim lngDecimal as Long
  lngDecimal = Clng(&H80000000)
  If lngDecimal = -2147483648 Then
     doUnitTest = 1
  Else
     doUnitTest = 0
  End If
End Function
