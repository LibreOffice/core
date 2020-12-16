Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testUCase
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testUCase()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test UCase function"
    On Error GoTo errorHandler

    date2 = "HELLO 12"
    date1 = UCase("hello 12") '2/12/1969
    TestUtilModule.AssertTrue(date1 = date2, "the return UCase is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

