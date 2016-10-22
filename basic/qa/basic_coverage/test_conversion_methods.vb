'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    Randomize 42
    dim aVector as Variant
    aVector = Array( Date(), Time(), "Hello", -3.14)
    ' CBOOL
    If (CBool(3) <> True) Then
        doUnitTest = 0
    ' CBYTE
    Else If (CByte("3") <> 3) Then
        doUnitTest = 0
    ' CCUR
    Else If (CCur("100") <> 100) Then
        doUnitTest = 0
    ' CDATE
    Else If (CDate(100) <> 100) Then
        doUnitTest = 0
    ' CDEC
    'Else If (CDec("100") <> 100) Then
    '    doUnitTest = 0
    ' CDBL
    Else If (CDbl("100") <> 100) Then
        doUnitTest = 0
    ' CINT
    Else If (CInt("100") <> 100) Then
        doUnitTest = 0
    ' CLNG
    Else If (CLng("100") <> 100) Then
        doUnitTest = 0
    ' CSNG
    Else If (CSng("100") <> 100) Then
        doUnitTest = 0
    ' CSTR
    Else If (CStr(100) <> "100") Then
        doUnitTest = 0
    ' VAL
    Else If ( Val("4") <> 4 ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    End If
    'End If
End Function
