Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testSwitch
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testSwitch()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Switch function"
    On Error GoTo errorHandler

    date2 = "French"
    date1 = MatchUp("Paris")
    TestUtilModule.AssertTrue(date1 = date2, "the return Switch is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

Function MatchUp(CityName As String)
    MatchUp = Switch(CityName = "London", "English", CityName _
                    = "Rome", "Italian", CityName = "Paris", "French")
End Function

