'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    If (Mid("abc", 5) = "") Then
        doUnitTest = 1
    Else
        doUnitTest = 0
    End If
End Function
