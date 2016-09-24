'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer

Dim s1 As String
Dim s2 As String

    s1 = "abc"

    s2 = Right(s1, 2)

    If s2 = "bc" Then
        doUnitTest = 1
    Else
        doUnitTest = 0
    End If

End Function
