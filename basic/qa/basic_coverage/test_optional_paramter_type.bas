' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Compatible
Option Explicit

Function doUnitTest() As String
    doUnitTest = "FAIL"
    If CheckType1(32) = 0 Then
        Exit Function
    End If
    If CheckType2(32) = 0 Then
        Exit Function
    End If
    If CheckType2() = 0 Then
        Exit Function
    End If
    doUnitTest = "OK"
End Function

Function CheckType1(x As Integer) As Integer
    If TypeName(x) = "Integer" Then
         CheckType1 = 1
    Else
         CheckType1 = 0
    End If
End Function


Function CheckType2(Optional y As Integer = 32 ) As Integer
    If TypeName(y) = "Integer" Then
        CheckType2 = 1
    Else
        CheckType2 = 0
    End If
End Function
