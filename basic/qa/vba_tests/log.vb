Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testLog
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testLog()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Log function"
    On Error GoTo errorHandler

    date2 = 4.454
    date1 = Log(86)
    TestUtilModule.AssertTrue(Round(date1, 3) = Round(date2, 3), "the return Log is: " & date1)

    date2 = 4
    date1 = Exp(Log(4))
    TestUtilModule.AssertTrue(date1 = date2, "the return Log is: " & date1)

    date2 = 1
    date1 = Log(2.7182818)
    TestUtilModule.AssertTrue(Round(date1, 3) = Round(date2, 3), "the return Log is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

