Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testLen
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testLen()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Len function"
    On Error GoTo errorHandler

    date2 = 8
    date1 = Len("sometext")
    TestUtilModule.AssertTrue(date1 = date2, "the return Len is: " & date1)

    date2 = 9
    date1 = Len("some text")
    TestUtilModule.AssertTrue(date1 = date2, "the return Len is: " & date1)

    date2 = 0
    date1 = Len("")
    TestUtilModule.AssertTrue(date1 = date2, "the return Len is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

