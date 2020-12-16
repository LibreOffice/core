Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testWeekDayName
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testWeekDayName()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test WeekDayName function"
    On Error GoTo errorHandler

    date2 = "Sunday"
    date1 = WeekdayName(1)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDayName is: " & date1)

    date2 = "Sunday"
    date1 = WeekdayName(1, , vbSunday)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDayName is: " & date1)

    date2 = "Monday"
    date1 = WeekdayName(1, , vbMonday)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDayName is: " & date1)

    date2 = "Monday"
    date1 = WeekdayName(2)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDayName is: " & date1)

    date2 = "Tue"
    date1 = WeekdayName(2, True, vbMonday)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDayName is: " & date1)

    date2 = "Wed"
    date1 = WeekdayName(2, True, vbTuesday)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDayName is: " & date1)

    date2 = "Thu"
    date1 = WeekdayName(2, True, vbWednesday)
    TestUtilModule.AssertTrue(date1 = date2, "the return WeekDayName is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

