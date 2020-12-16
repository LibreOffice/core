Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testSQR
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testSQR()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test SQR function"
    On Error GoTo errorHandler

    date2 = 3
    date1 = Sqr(9)
    TestUtilModule.AssertTrue(date1 = date2, "the return SQR is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

