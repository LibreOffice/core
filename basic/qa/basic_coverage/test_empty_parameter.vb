'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Sub assignVar(v As Variant)
    v = 1
End Sub

Function doUnitTest() As Integer
    ' tdf#132563 - check if empty parameters are converted to their respective types
    anEmptyVar = Empty
    assignVar(anEmptyVar)
    If (anEmptyVar = 1 And TypeName(anEmptyVar) = "Integer") Then
        doUnitTest = 1
    Else
        doUnitTest = 0
    End If
End Function
