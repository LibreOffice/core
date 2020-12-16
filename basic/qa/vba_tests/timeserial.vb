Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testTimeSerial
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testTimeSerial()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2 As Date
    testName = "Test TimeSerial function"
    On Error GoTo errorHandler

rem    bug 114229
rem    date2 = "5:45:00"
rem    date1 = (TimeSerial(12 - 6, -15, 0))
rem    TestUtilModule.AssertTrue(date1 = date2, "the return TimeSerial is: " & date1)

    date2 = "12:30:00"
    date1 = TimeSerial(12, 30, 0)
    TestUtilModule.AssertTrue(date1 = date2, "the return TimeSerial is: " & date1)

rem    date2 = "11:30:00"
rem    date1 = TimeSerial(10, 90, 0)
rem    TestUtilModule.AssertTrue(date1 = date2, "the return TimeSerial is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

