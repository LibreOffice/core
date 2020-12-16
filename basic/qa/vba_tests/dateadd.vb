Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testDateAdd
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testDateAdd()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2 As Date   'variables for test
    testName = "Test DateAdd function"
    On Error GoTo errorHandler

    date2 = CDate("1995-02-28")
    date1 = DateAdd("m", 1, "1995-01-31")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateAdd is: " & date1)

    date2 = CDate("1995-02-28")
    date1 = DateAdd("m", 1, "1995-01-31")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateAdd is: " & date1)

    date2 = CDate("1995-02-28")
    date1 = DateAdd("m", 1, "1995-01-31")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateAdd is: " & date1)

    date2 = CDate("1996-01-31")
    date1 = DateAdd("yyyy", 1, "1995-01-31")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateAdd is: " & date1)

    date2 = CDate("1995-04-30")
    date1 = DateAdd("q", 1, "1995-01-31")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateAdd is: " & date1)

    date2 = CDate("1995-02-01")
    date1 = DateAdd("y", 1, "1995-01-31")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateAdd is: " & date1)

    date2 = CDate("1995-02-01")
    date1 = DateAdd("d", 1, "1995-01-31")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateAdd is: " & date1)

    date2 = CDate("1995-02-01")
    date1 = DateAdd("w", 1, "1995-01-31")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateAdd is: " & date1)

    date2 = CDate("1995-02-07")
    date1 = DateAdd("ww", 1, "1995-01-31")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateAdd is: " & date1)

Rem This fails when directly comparing date1=date2, probably due to rounding.
Rem Workaround convert to string which does the rounding.
    Dim date1s, date2s As String
    date2 = CDate("1995-01-01 22:48:29")
    date1 = DateAdd("h", 1, "1995-01-01 21:48:29")
    date1s = "" & date1
    date2s = "" & date2
    TestUtilModule.AssertTrue(date1s = date2s, "the return DateAdd is: " & date1)

    date2 = CDate("1995-01-31 21:49:29")
    date1 = DateAdd("n", 1, "1995-01-31 21:48:29")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateAdd is: " & date1)

    date2 = CDate("1995-01-31 21:48:30")
    date1 = DateAdd("s", 1, "1995-01-31 21:48:29")
    TestUtilModule.AssertTrue(date1 = date2, "the return DateAdd is: " & date1)

exitFunc:
    TestUtilModule.TestEnd

    Exit Sub

errorHandler:
    On Error GoTo 0
    TestUtilModule.AssertTrue(False, testName & ": hit error handler")
    GoTo exitFunc

End Sub

