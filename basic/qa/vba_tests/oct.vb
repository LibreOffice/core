Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testOct
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testOct()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Oct function"
    On Error GoTo errorHandler

    date2 = 4
    date1 = Oct(4)
    TestUtilModule.AssertTrue(date1 = date2, "the return Oct is: " & date1)

    date2 = 10
    date1 = Oct(8)
    TestUtilModule.AssertTrue(date1 = date2, "the return Oct is: " & date1)

    date2 = 713
    date1 = Oct(459)
    TestUtilModule.AssertTrue(date1 = date2, "the return Oct is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

