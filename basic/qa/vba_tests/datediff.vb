Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testDateDiff
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testDateDiff()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1
    Dim date2
    testName = "Test DateDiff function"
    On Error GoTo errorHandler

    date2 = 10
    date1 = DateDiff("yyyy", "22/11/2003", "22/11/2013")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 40
    date1 = DateDiff("q", "22/11/2003", "22/11/2013")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 120
    date1 = DateDiff("m", "22/11/2003", "22/11/2013")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3653
    date1 = DateDiff("y", "22/11/2003", "22/11/2013")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 521
    date1 = DateDiff("w", "22/11/2003", "22/11/2013")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 522
    date1 = DateDiff("ww", "22/11/2003", "22/11/2013")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 87672
    date1 = DateDiff("h", "22/11/2003", "22/11/2013")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 525600
    date1 = DateDiff("n", "22/11/2012", "22/11/2013")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 2678400
    date1 = DateDiff("s", "22/10/2013", "22/11/2013")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbFriday)
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbMonday)
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3623
    date1 = DateDiff("d", "22/12/2003", "22/11/2013", vbSaturday)
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3684
    date1 = DateDiff("d", "22/10/2003", "22/11/2013", vbSunday)
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbThursday)
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbTuesday)
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbFriday, vbFirstJan1)
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbThursday, vbFirstFourDays)
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbSunday, vbFirstFullWeek)
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    date2 = 3653
    date1 = DateDiff("d", "22/11/2003", "22/11/2013", vbSaturday, vbFirstFullWeek)
    TestUtilModule.AssertTrue(date1 = date2, "the return DateDiff is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

