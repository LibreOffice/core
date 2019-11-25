'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as Integer
    ' Verify that in the default locale the comma is the group separator
    ' and the period is the decimal separator.
     If (CSng("78,100.123") <> 78100.123) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
