' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest As String
    ' Trying to create too long string should generate proper BASIC overflow error.
    ' Longest possible string is 2147483638 wchar_t (2G - 10).
    ' This tries to create string with 2G wchar_t. If it does not overflow, test fails.
    ' If overflow is not safe, it segfaults.
    On Error GoTo errorHandler
    Dim s As String, i As Integer
    s = "0"
    For i=1 To 31
    s = s & s
    Next i
    doUnitTest = "FAIL"
    Exit Function
errorHandler:
    If ( Err <> 6 ) Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    Endif
End Function
