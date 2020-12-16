Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testSTR
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testSTR()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test STR function"
    On Error GoTo errorHandler

    date2 = " 459"
    date1 = Str(459)
    TestUtilModule.AssertTrue(date1 = date2, "the return STR is: " & date1)

    date2 = "-459.65"
    date1 = Str(-459.65)
    TestUtilModule.AssertTrue(date1 = date2, "the return STR is: " & date1)

    date2 = " 459.001"
    date1 = Str(459.001)
    TestUtilModule.AssertTrue(date1 = date2, "the return STR is: " & date1)

    date2 = " .24"
    date1 = Str(0.24)
    TestUtilModule.AssertTrue(date1 = date2, "the return STR is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

