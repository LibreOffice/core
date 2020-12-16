Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testExp
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testExp()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Exp function"
    On Error GoTo errorHandler

    date2 = 2.7183
    date1 = Exp(1)
    TestUtilModule.AssertTrue(Round(date1, 4) = Round(date2, 4), "the return Exp is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

