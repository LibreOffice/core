Function doUnitTest() as Integer
    Dim A As Double
    ' In the da-DK locale the comma is the decimal separator and the
    ' period is the grouping separator. The CDbl function is
    ' expected to use the locale. Verify that.
    A = CDbl("222.222")
    If A <> 222222 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
