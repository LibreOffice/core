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
    ElseIf (CByte("3") <> 3) Then
        doUnitTest = 0
    ' CCUR
    ElseIf (CCur("100") <> 100) Then
        doUnitTest = 0
    ' CDATE
    ElseIf (CDate(100) <> 100) Then
        doUnitTest = 0
    ' CDEC already tested
    'ElseIf (CDec("100") <> 100) Then
    '    doUnitTest = 0
    ' CDBL
    ElseIf (CDbl("100") <> 100) Then
        doUnitTest = 0
    ' CINT
    ElseIf (CInt("100") <> 100) Then
        doUnitTest = 0
    ' CLNG already tested
    'ElseIf (CLng("100") <> 100) Then
     '   doUnitTest = 0
    ' CSNG
    ElseIf (CSng("100") <> 100) Then
        doUnitTest = 0
    ' CSTR
    ElseIf (CStr(100) <> "100") Then
        doUnitTest = 0
    ' VAL
    ElseIf ( Val("4") <> 4 ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
