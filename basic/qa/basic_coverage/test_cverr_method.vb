'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    ' CVERR
    If (CVerr(100) <> 100) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If

    ' tdf#79426 - passing an error object to a function
    if ( TestCVErr( CVErr( 2 ) ) <> 2 ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If

    ' tdf#79426 - test with Error-Code 448 ( ERRCODE_BASIC_NAMED_NOT_FOUND )
    if ( TestCVErr( CVErr( 448 ) ) <> 448 ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function

Function TestCVErr(vErr As Variant)
    Dim nValue As Integer
    nValue = vErr
    TestCVErr = nValue
End Function