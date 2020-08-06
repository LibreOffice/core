'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    Dim n As Integer, i As Integer
    n = 0
    For Each i In Array(0)
        n = n + 1
    Next i
    If n = 1 Then
        doUnitTest = 1
    Else
        doUnitTest = 0
    End If
End Function
