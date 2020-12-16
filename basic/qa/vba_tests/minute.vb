Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testMinute
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testMinute()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Minute function"
    On Error GoTo errorHandler

    date2 = 34
    date1 = Minute("09:34:20")
    TestUtilModule.AssertTrue(date1 = date2, "the return Minute is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

