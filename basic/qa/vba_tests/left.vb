Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testLeft
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testLeft()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Left function"
    On Error GoTo errorHandler

    date2 = "some"
    date1 = Left("sometext", 4)
    TestUtilModule.AssertTrue(date1 = date2, "the return Left is: " & date1)

    date2 = "sometext"
    date1 = Left("sometext", 48)
    TestUtilModule.AssertTrue(date1 = date2, "the return Left is: " & date1)

    date2 = ""
    date1 = Left("", 4)
    TestUtilModule.AssertTrue(date1 = date2, "the return Left is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

