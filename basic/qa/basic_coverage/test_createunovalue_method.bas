' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
    ' CreateUnoValue

    On Error GoTo errorHandler

    Dim oUnoValue as Variant
    Dim aValue as Variant
    aValue = Array ( 1, 1 )
    oUnoValue = CreateUnoValue( "[]byte", aValue )

    ' tdf#148063: Without the fix in place, this test would have crashed
    oUnoValue = CreateUnoValue( "[]", aValue )
    doUnitTest = "OK"

errorHandler:
    If ( Err <> 1 ) Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    Endif
End Function
