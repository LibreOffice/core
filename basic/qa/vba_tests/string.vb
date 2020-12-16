Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testString
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testString()

    TestUtilModule.TestInit

    Dim testName As String
    Dim TestStr As String
    Dim date1, date2
    testName = "Test String function"
    On Error GoTo errorHandler

    date2 = "PPPPP"
    date1 = String(5, "P")
    TestUtilModule.AssertTrue(date1 = date2, "the return String is: " & date1)

    date2 = "aaaaa"
    date1 = String(5, "a")
    TestUtilModule.AssertTrue(date1 = date2, "the return String is: " & date1)

    date2 = ""
    date1 = String(0, "P")
    TestUtilModule.AssertTrue(date1 = date2, "the return String is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

