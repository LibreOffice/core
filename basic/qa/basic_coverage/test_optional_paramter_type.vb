REM  *****  BASIC  *****
Option Compatible

Function doUnitTest() As Integer
    doUnitTest = 0
    If CheckType1(32) = 0 Then
        Exit Function
    End If
    If CheckType2(32) = 0 Then
        Exit Function
    End If
    If CheckType2() = 0 Then
        Exit Function
    End If
    doUnitTest = 1
End Function

Function CheckType1(x As Integer) As Integer
    If TypeName(x) = "Integer" Then
         CheckType1 = 1
    Else
         CheckType1 = 0
    End If
End Function


Function CheckType2(Optional y As Integer = 32 ) As Integer
    If TypeName(y) = "Integer" Then
        CheckType2 = 1
    Else
        CheckType2 = 0
    End If
End Function