Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testFix
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testFix()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Fix function"
    On Error GoTo errorHandler

    date2 = 12
    date1 = Fix(12.34)
    TestUtilModule.AssertTrue(date1 = date2, "the return Fix is: " & date1)

    date2 = 12
    date1 = Fix(12.99)
    TestUtilModule.AssertTrue(date1 = date2, "the return Fix is: " & date1)

    date2 = -8
    date1 = Fix(-8.4)
    TestUtilModule.AssertTrue(date1 = date2, "the return Fix is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

