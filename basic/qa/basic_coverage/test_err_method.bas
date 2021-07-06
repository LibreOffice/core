' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
    ' ERR
    On Error GoTo ErrorHandler ' Set up error handler
    Dim nVar As Integer
    nVar = 0
    nVar = 1/nVar
    doUnitTest = "FAIL"
    Exit Function
ErrorHandler:
    If ( Err <> 11 ) Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    Endif
End Function
