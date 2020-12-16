Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testHour
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testHour()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2, myTime
    testName = "Test Hour function"
    On Error GoTo errorHandler
    
    myTime = "6:25:39 AM"
    date2 = 6
    date1 = Hour(myTime)
    TestUtilModule.AssertTrue(date1 = date2, "the return Hour is: " & date1)

    myTime = "6:25:39 PM"
    date2 = 18
    date1 = Hour(myTime)
    TestUtilModule.AssertTrue(date1 = date2, "the return Hour is: " & date1)

    myTime = "06:25:39 AM"
    date2 = 6
    date1 = Hour(myTime)
    TestUtilModule.AssertTrue(date1 = date2, "the return Hour is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

