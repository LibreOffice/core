Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testCDate
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testCDate()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2 As Date   'variables for test
    testName = "Test CDate function"
    On Error GoTo errorHandler

    date2 = 25246
    date1 = CDate("12/02/1969") '02/12/1969
    TestUtilModule.AssertTrue(date1 = date2, "the return CDate is: " & date1)

    date2 = 28313
    date1 = CDate("07/07/1977")
    TestUtilModule.AssertTrue(date1 = date2, "the return CDate is: " & date1)

    date2 = 28313
    date1 = CDate(#7/7/1977#)
    TestUtilModule.AssertTrue(date1 = date2, "the return CDate is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

