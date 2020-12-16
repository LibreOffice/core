Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testRight
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testRight()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Right function"
    On Error GoTo errorHandler

    date2 = "text"
    date1 = Right("sometext", 4)
    TestUtilModule.AssertTrue(date1 = date2, "the return Right is: " & date1)

    date2 = "sometext"
    date1 = Right("sometext", 48)
    TestUtilModule.AssertTrue(date1 = date2, "the return Right is: " & date1)

    date2 = ""
    date1 = Right("", 4)
    TestUtilModule.AssertTrue(date1 = date2, "the return Right is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

