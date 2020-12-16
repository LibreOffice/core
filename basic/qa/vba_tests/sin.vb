Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testSIN
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testSIN()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test SIN function"
    On Error GoTo errorHandler

    date2 = 0.43
    date1 = Sin(0.45)
    TestUtilModule.AssertTrue(Round(date1, 2) = Round(date2, 2), "the return SIN is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

