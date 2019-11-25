'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as Integer
    ' Verify that in the default locale the period is the decimal separator,
    ' and that CInt rounds to nearest integer.
    If (CInt("100.789") <> 101) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
