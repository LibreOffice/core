'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    dim aString as Variant
    aString = "Hello"
    ' ASC
    If (Asc("€") <> 8364) Then
        doUnitTest = 0
    ' ASCw
    'ElseIf (AscW("€") <> 8364) Then
    '    doUnitTest = 0
    ' FORMAT already tested
    ' ElseIf (Format(PI, "0") <> "3") Then
    '    doUnitTest = 0
    ' InStr
    ElseIf ( InStr( 1, aString, "l", 1) <> 3 ) Then
        doUnitTest = 0
    ' InStrRev
    'ElseIf ( InStrRev( aString, "l" ) <> 4 ) Then
    '    doUnitTest = 0
    ' LCASE
    ElseIf ( LCase( aString ) <> "hello" ) Then
        doUnitTest = 0
    ' LEFT already test
    ' ElseIf ( Left( aString, 2 ) <> "He" ) Then
    '    doUnitTest = 0
    ' LEN
    ElseIf ( Len( aString ) <> 5 ) Then
        doUnitTest = 0
    ' LENB
    ElseIf ( LenB( aString ) <> 5 ) Then
        doUnitTest = 0
    ' LTRIM
    ElseIf ( LTrim( "  Hello" ) <> aString ) Then
        doUnitTest = 0
    ' MID already tested
    ' ElseIf ( Mid( aString, 2, 2 ) <> "el" ) Then
    '    doUnitTest = 0
    ' REPLACE already tested
    ' ElseIf ( Replace ( aString, "l", "jo", 1, 2 ) <> "Hejojoo" ) Then
    '    doUnitTest = 0
    ' RIGHT already tested
    ' ElseIf ( Right( aString, 2 ) <> "lo" ) Then
    '    doUnitTest = 0
    ' RTRIM
    ElseIf ( RTrim( "Hello  " ) <> aString ) Then
        doUnitTest = 0
    ' SPACE
    ElseIf ( Space(3) <> "   " ) Then
        doUnitTest = 0
    ' SPC
    ElseIf ( Spc(3) <> "   " ) Then
        doUnitTest = 0
    ' SPLIT
    ElseIf ( Split( "Hello world" )(1) <> "world" ) Then
        doUnitTest = 0
    ' STR TRIM
    ElseIf ( Trim( Str( 4 ) ) <> "4" ) Then
        doUnitTest = 0
    ' STRCOMP
    ElseIf ( StrComp( aString, "Hello" ) <> 0 ) Then
        doUnitTest = 0
    ' StrConv
    ' STRING
    ElseIf ( String( 3, "H" ) <> "HHH" ) Then
        doUnitTest = 0
    ' STRREVERSE
    'ElseIf ( StrReverse( aString ) <> "olleH" ) Then
    '    doUnitTest = 0
    ' TAB
    ElseIf ( "Hello" & Tab(0) & "World" <> "HelloWorld" ) Then
        doUnitTest = 0
    ' UCASE
    ElseIf ( UCase( aString ) <> "HELLO" ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
