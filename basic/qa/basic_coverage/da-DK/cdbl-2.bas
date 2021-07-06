Option Explicit

Function doUnitTest() as String
    Dim A As String
    Dim B As Double
    Dim Expected As Double
    A = "222,222"
    ' in da-DK locale ',' is the decimal separator
    Expected = 222.222
    B = Cdbl(A)
    If B <> Expected Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    End If
End Function
