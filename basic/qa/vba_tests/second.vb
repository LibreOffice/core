Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testSecond
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testSecond()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Second function"
    On Error GoTo errorHandler

    date2 = 0
    date1 = Second(37566.3)
    TestUtilModule.AssertTrue(date1 = date2, "the return Second is: " & date1)

    date2 = 17
    date1 = Second("4:35:17")
    TestUtilModule.AssertTrue(date1 = date2, "the return Second is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

