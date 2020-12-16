Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testIsDate
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testIsDate()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test IsDate function"
    On Error GoTo errorHandler

    date2 = True
    date1 = IsDate(cdate("12/2/1969"))
    TestUtilModule.AssertTrue(date1 = date2, "the return IsDate is: " & date1)

    date2 = True
    date1 = IsDate("12:22:12")
    TestUtilModule.AssertTrue(date1 = date2, "the return IsDate is: " & date1)

    date2 = False
    date1 = IsDate("a12.2.1969")
    TestUtilModule.AssertTrue(date1 = date2, "the return IsDate is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

