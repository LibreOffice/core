' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    ' ERR
    On Error GoTo ErrorHandler ' Set up error handler
    Dim nVar As Integer
    nVar = 0
    nVar = 1/nVar
    doUnitTest = 0
    Exit Function
ErrorHandler:
    If ( Err <> 11 ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    Endif
End Function
