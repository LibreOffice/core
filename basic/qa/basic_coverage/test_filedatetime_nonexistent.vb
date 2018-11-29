' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'
'Bug 121337 - FileDateTime("\\nonexistent\smb\path") returns bogus result rather than throwing error
Function doUnitTest as Integer
    On Error GoTo ErrorHandler ' Set up error handler
    Dim result
    result = FileDateTime("/bogus/unix/path")
    doUnitTest = 0
    Exit Function
ErrorHandler:
    If ( Err <> 0 ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    Endif
End Function
