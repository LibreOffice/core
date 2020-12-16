Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testYear
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testYear()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Year function"
    On Error GoTo errorHandler

    date2 = 1969
    date1 = Year("12/2/1969")
    TestUtilModule.AssertTrue(date1 = date2, "the return Year is: " & date1)

    date2 = 1900
    date1 = Year(256)
    TestUtilModule.AssertTrue(date1 = date2, "the return Year is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

