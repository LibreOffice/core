Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testday
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testday()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2   'variables for test
    testName = "Test day function"
    On Error GoTo errorHandler

    date2 = 12
    date1 = Day("1969-02-12") '2/12/1969
    TestUtilModule.AssertTrue(date1 = date2, "the return day is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

