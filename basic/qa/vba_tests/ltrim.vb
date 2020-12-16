Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testLTrim
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testLTrim()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test LTrim function"
    On Error GoTo errorHandler

    date2 = "some text  "
    date1 = LTrim("   some text  ")
    TestUtilModule.AssertTrue(date1 = date2, "the return LTrim is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

