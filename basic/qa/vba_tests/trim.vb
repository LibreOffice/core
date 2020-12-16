Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testTrim
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testTrim()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Trim function"
    On Error GoTo errorHandler

    date2 = "some text"
    date1 = Trim("   some text  ")
    TestUtilModule.AssertTrue(date1 = date2, "the return Trim is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

