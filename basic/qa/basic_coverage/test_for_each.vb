'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    Dim n As Integer, i
    Dim a(3)
    n = 0
    For Each i In a
        n = n + 1
    Next i
    If n <> 4 Then
        doUnitTest = "For Each over array failed"
        Exit Function
    End If

    If TestInvalidForEachWithErrorHandler <> "13 91 14 " Then
        doUnitTest = "For Each doesn't generate proper errors on bad arguments"
        Exit Function
    End If

    doUnitTest = 1
End Function

Function TestInvalidForEachWithErrorHandler
    Dim s As String
    On Error Goto ErrHandler
' This For Each is given a bad iterable; it must generate first error ("Data type mismatch") for b;
    For Each a In b
' Then proceed here (Resume Next from ErrHandler), and generate "Object variable not set" for c;
    c.d
' Then proceed here (Resume Next from ErrHandler), and generate "Invalid parameter" at Next.
    Next
    TestInvalidForEachWithErrorHandler = s
    Exit Function
ErrHandler:
    s = s & Err & " "
    Resume Next
End Function
