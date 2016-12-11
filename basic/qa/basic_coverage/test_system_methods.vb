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
    'dim aDate as Variant
    'aDate = Date()
    'WaitUntil( aDate )
    ' CompatibilityMode
    If (CompatibilityMode(True) <> True) Then
        doUnitTest = 0
    ' ConvertFromUrl ConvertToUrl
    ElseIf ( ConvertToUrl( ConvertFromUrl("") ) <> "") Then
        doUnitTest = 0
    ' GetSolarVersion
    ElseIf ( GetSolarVersion() < 50000) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
