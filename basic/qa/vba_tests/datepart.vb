Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testDatePart
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testDatePart()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1   'variables for test
    Dim date2
    testName = "Test DatePart function"
    On Error GoTo errorHandler

    date2 = 1969
    date1 = DatePart("yyyy", "1969-02-12")
    TestUtilModule.AssertTrue(date1 = date2, "the return DatePart is: " & date1)

    date2 = 1
    date1 = DatePart("q", "1969-02-12")
    TestUtilModule.AssertTrue(date1 = date2, "the return DatePart is: " & date1)

    date2 = 43
    date1 = DatePart("y", "1969-02-12")
    TestUtilModule.AssertTrue(date1 = date2, "the return DatePart is: " & date1)

    date2 = 12
    date1 = DatePart("d", "1969-02-12")
    TestUtilModule.AssertTrue(date1 = date2, "the return DatePart is: " & date1)

    date2 = 4
    date1 = DatePart("w", "1969-02-12")
    TestUtilModule.AssertTrue(date1 = date2, "the return DatePart is: " & date1)

    date2 = 7
    date1 = DatePart("ww", "1969-02-12")
    TestUtilModule.AssertTrue(date1 = date2, "the return DatePart is: " & date1)

    date2 = 16
    date1 = DatePart("h", "1969-02-12 16:32:00")
    TestUtilModule.AssertTrue(date1 = date2, "the return DatePart is: " & date1)

    date2 = 32
    date1 = DatePart("n", "1969-02-12 16:32:00")
    TestUtilModule.AssertTrue(date1 = date2, "the return DatePart is: " & date1)

    date2 = 0
    date1 = DatePart("s", "1969-02-12 16:32:00")
    TestUtilModule.AssertTrue(date1 = date2, "the return DatePart is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

