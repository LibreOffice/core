Function doUnitTest() as Integer
    Dim A As Double
    ' In the da-DK locale ',' is the decimal separator and the CDbl
    ' function is expected to use the locale.
    A = CDbl("222,222")
    If A <> 222.222 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
