Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testRTrim
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testRTrim()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test RTrim function"
    On Error GoTo errorHandler

    date2 = "   some text"
    date1 = RTrim("   some text  ")
    TestUtilModule.AssertTrue(date1 = date2, "the return RTrim is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

