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
    ' ABS
    If (Abs(-3.5) <> 3.5) Then
        doUnitTest = 0
    Else If (Abs(3.5) <> 3.5) Then
        doUnitTest = 0
    ' ATN
    Else If (Atn(1) <> PI/4) Then
        doUnitTest = 0
    ' COS SIN
    Else If ( Abs(Cos(PI/3) - Sin(PI/6)) > 1E-6 ) Then
        doUnitTest = 0
    ' EXP LOG
    Else If ( Log( Exp(1) ) <> 1 ) Then
        doUnitTest = 0
    ' FIX
    Else If (Fix(PI) <> 3) Then
        doUnitTest = 0
    ' FRAC
    Else If ( 3+Frac(PI) <> PI) Then
        doUnitTest = 0
    ' HEX
    Else If ( Hex(100) <> "64") Then
        doUnitTest = 0
    ' INT
    Else If ( Int(PI) <> 3 ) Then
        doUnitTest = 0
    ' OCT
    Else If ( Oct(100) <> "144" ) Then
        doUnitTest = 0
    ' ROUND
    ' Else If ( Round( PI, 2 ) <> 3.14 ) Then
    '    doUnitTest = 0
    ' RND
    Else If ( Rnd >= 1 ) Then
        doUnitTest = 0
    ' SGN
    Else If ( Sgn(aVector(3)) <> -1 ) Then
        doUnitTest = 0
    ' SQR
    Else If ( Sqr( 4 ) <> 2 ) Then
        doUnitTest = 0
    ' TAN
    Else If ( Abs( Tan(PI/4) - 1 ) > 1E-6 ) Then
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
    'End If
End Function
