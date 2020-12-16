Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testWeekDay
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testWeekDay()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test WeekDay function"
    On Error GoTo errorHandler

    date2 = 7
    date1 = Weekday(#6/7/2009#, vbMonday)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDay is: " & date1)

    date2 = 2
    date1 = Weekday(#7/7/2009#, vbMonday)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDay is: " & date1)

    date2 = 5
    date1 = Weekday(#8/7/2009#, vbMonday)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDay is: " & date1)

    date2 = 1
    date1 = Weekday(#12/7/2009#, vbMonday)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDay is: " & date1)

    date2 = 1
    date1 = Weekday(#6/7/2009#, vbSunday)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDay is: " & date1)

    date2 = 5
    date1 = Weekday(#6/7/2009#, 4)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDay is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

