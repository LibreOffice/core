Option Explicit

Function doUnitTest As Integer
    ' Trying to create too long string should generate proper BASIC overflow error.
    ' Longest possible string is 2147483638 wchar_t (2G - 10).
    ' This tries to create string with 2G wchar_t. If it does not overflow, test fails.
    ' If overflow is not safe, it segfaults.
    On Error GoTo errorHandler
    Dim s As String, i As Integer
    s = "0"
    For i=1 To 31
    s = s & s
    Next i
    doUnitTest = 0
    Exit Function
errorHandler:
    If ( Err <> 6 ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    Endif
End Function
