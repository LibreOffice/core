'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    Wait(0)
    Beep
    'dim aVector as Variant
    'aVector = Array( Date(), Time(), "Hello", -3.14)
    'WaitUntil( aVector(0) )
    ' CompatibilityMode
    If (CompatibilityMode(True) <> True) Then
        doUnitTest = 0
    ' ConvertFromUrl ConvertToUrl
    Else If ( ConvertToUrl( ConvertFromUrl("") ) <> "") Then
        doUnitTest = 0
    ' GetSolarVersion
    Else If ( GetSolarVersion() < 50000) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
    End If
    End If
End Function
