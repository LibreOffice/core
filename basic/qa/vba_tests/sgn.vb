Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_SGN
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_SGN()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test SGN function"
    On Error GoTo errorHandler

    date2 = 0
    date1 = sgn(0)
    TestUtilModule.AssertTrue(date1 = date2, "the return SGN is: " & date1)

    date2 = -1
    date1 = sgn(-1)
    TestUtilModule.AssertTrue(date1 = date2, "the return SGN is: " & date1)

    date2 = 1
    date1 = sgn(1)
    TestUtilModule.AssertTrue(date1 = date2, "the return SGN is: " & date1)

    date2 = 1
    date1 = sgn(50)
    TestUtilModule.AssertTrue(date1 = date2, "the return SGN is: " & date1)

    date2 = -1
    date1 = sgn(-50)
    TestUtilModule.AssertTrue(date1 = date2, "the return SGN is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

