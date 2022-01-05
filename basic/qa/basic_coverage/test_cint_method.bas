'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
    ' CINT
    If (CInt(-258.0427e+2 + 133*2 + &h64 -&o220) <> -25582) Or _
       (CInt(expression := #1990-10-27) <> 1953) Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    End If
End Function
