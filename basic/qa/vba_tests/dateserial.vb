Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testDateSerial
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testDateSerial()
    Dim testName As String
    Dim date1, date2 As Date

    TestUtilModule.TestInit

    testName = "Test DateSerial function"
    date2 = 36326

    On Error GoTo errorHandler

    date1 = DateSerial(1999, 6, 15)   '6/15/1999
    TestUtilModule.AssertTrue(date1 = date2, "the return date is: " & date1)
    date1 = DateSerial(2000, 1 - 7, 15) '6/15/1999
    TestUtilModule.AssertTrue(date1 = date2, "the return date is: " & date1)
    date1 = DateSerial(1999, 1, 166)  '6/15/1999
    TestUtilModule.AssertTrue(date1 = date2, "the return date is: " & date1)
    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
    TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

