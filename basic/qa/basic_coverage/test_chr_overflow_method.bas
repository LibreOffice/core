' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.

Option Explicit

Function overflow1 as Integer
    On Error GoTo handler
    Chr(-32769)
    overflow1 = 0
    Exit Function
handler:
    if (Err <> 6) Then
        overflow1 = 0
        Exit Function
    Endif
    overflow1 = 1
End Function

Function overflow2 as Integer
    On Error GoTo handler
    Chr(65536)
    overflow2 = 0
    Exit Function
handler:
    if (Err <> 6) Then
        overflow2 = 0
        Exit Function
    Endif
    overflow2 = 1
End Function

Function overflow3 as Integer
    On Error GoTo handler
    Chr(&H10000)
    overflow3 = 0
    Exit Function
handler:
    if (Err <> 6) Then
        overflow3 = 0
        Exit Function
    Endif
    overflow3 = 1
End Function

Function doUnitTest as String
    Chr(-32768)
    Chr(65535)
    Chr(&H8000)
    Chr(&HFFFF)
    if (overflow1 = 0) Then
        doUnitTest = "FAIL"
        Exit Function
    Endif
    if (overflow2 = 0) Then
        doUnitTest = "FAIL"
        Exit Function
    Endif
    if (overflow3 = 0) Then
        doUnitTest = "FAIL"
        Exit Function
    Endif
    doUnitTest = "OK"
End Function
