' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    ' Join
    Dim aStrings(2) as String
    aStrings(0) = "Hello"
    aStrings(1) = "world"
    If ( Join( aStrings, " " ) <> "Hello world " ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
