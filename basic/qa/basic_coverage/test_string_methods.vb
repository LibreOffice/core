'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    dim aVector as Variant
    aVector = Array( Date(), Time(), "Hello", -3.14)
    ' ASC
    If (Asc("€") <> 8364) Then
        doUnitTest = 0
    ' ASCw
    'Else If (AscW("€") <> 8364) Then
    '    doUnitTest = 0
    ' FORMAT
    Else If (Format(PI, "0") <> "3") Then
        doUnitTest = 0
    ' InStr
    Else If ( InStr( 1, aVector(2), "l", 1) <> 3 ) Then
        doUnitTest = 0
    ' InStrRev
    'Else If ( InStrRev( aVector(2), "l" ) <> 4 ) Then
    '    doUnitTest = 0
    ' LCASE
    Else If ( LCase( aVector(2) ) <> "hello" ) Then
        doUnitTest = 0
    ' LEFT
    Else If ( Left( aVector(2), 2 ) <> "He" ) Then
        doUnitTest = 0
    ' LEN
    Else If ( Len( aVector(2) ) <> 5 ) Then
        doUnitTest = 0
    ' LENB
    Else If ( LenB( aVector(2) ) <> 5 ) Then
        doUnitTest = 0
    ' LTRIM
    Else If ( LTrim( "  Hello" ) <> aVector(2) ) Then
        doUnitTest = 0
    ' MID
    Else If ( Mid( aVector(2), 2, 2 ) <> "el" ) Then
        doUnitTest = 0
    ' REPLACE
    Else If ( Replace ( aVector(2), "l", "jo", 1, 2 ) <> "Hejojoo" ) Then
        doUnitTest = 0
    ' RIGHT
    Else If ( Right( aVector(2), 2 ) <> "lo" ) Then
        doUnitTest = 0
    ' RTRIM
    Else If ( RTrim( "Hello  " ) <> aVector(2) ) Then
        doUnitTest = 0
    ' SPACE
    Else If ( Space(3) <> "   " ) Then
        doUnitTest = 0
    ' SPC
    Else If ( Spc(3) <> "   " ) Then
        doUnitTest = 0
    ' SPLIT
    Else If ( Split( "Hello world" )(1) <> "world" ) Then
        doUnitTest = 0
    ' STR TRIM
    Else If ( Trim( Str( 4 ) ) <> "4" ) Then
        doUnitTest = 0
    ' STRCOMP
    Else If ( StrComp( aVector(2), "Hello" ) <> 0 ) Then
        doUnitTest = 0
    ' StrConv
    ' STRING
    Else If ( String( 3, "H" ) <> "HHH" ) Then
        doUnitTest = 0
    ' STRREVERSE
    'Else If ( StrReverse( aVector(2) ) <> "olleH" ) Then
    '    doUnitTest = 0
    ' TAB
    Else If ( "Hello" & Tab(0) & "World" <> "HelloWorld" ) Then
        doUnitTest = 0
    ' UCASE
    Else If ( UCase( aVector(2) ) <> "HELLO" ) Then
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
    'End If
    'End If
End Function
