Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testMonthName
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testMonthName()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2 
    testName = "Test MonthName function"
    On Error GoTo errorHandler

    date2 = "February"
    date1 = MonthName(2)
    TestUtilModule.AssertTrue(date1 = date2, "the return MonthName is: " & date1)

    date2 = "Feb"
    date1 = MonthName(2, True)
    TestUtilModule.AssertTrue(date1 = date2, "the return MonthName is: " & date1)

    date2 = "February"
    date1 = MonthName(2, False)
    TestUtilModule.AssertTrue(date1 = date2, "the return MonthName is: " & date1)
    
    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

