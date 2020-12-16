Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testIsError
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testIsError()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test IsError function"
    On Error GoTo errorHandler

    date2 = False
    date1 = IsError("12.2.1969")
    TestUtilModule.AssertTrue(date1 = date2, "the return IsError is: " & date1)
    
    date2 = True
    date1 = IsError(CVErr(64))
    TestUtilModule.AssertTrue(date1 = date2, "the return IsError is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

