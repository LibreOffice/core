Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testMonth
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testMonth()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    Dim ldate As Date
    testName = "Test Month function"
    On Error GoTo errorHandler
    ldate = 32616

    date2 = 4
    date1 = Month(ldate)
    TestUtilModule.AssertTrue(date1 = date2, "the return Month is: " & date1)

    date2 = 2
    date1 = Month("01/02/2007")
    TestUtilModule.AssertTrue(date1 = date2, "the return Month is: " & date1)

    date2 = 12
    date1 = Month(1)
    TestUtilModule.AssertTrue(date1 = date2, "the return Month is: " & date1)

    date2 = 2
    date1 = Month(60)
    TestUtilModule.AssertTrue(date1 = date2, "the return Month is: " & date1)

    date2 = 6
    date1 = Month(2000)
    TestUtilModule.AssertTrue(date1 = date2, "the return Month is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

