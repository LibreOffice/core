Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testTimeValue
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testTimeValue()

    TestUtilModule.TestInit

    Dim testName As String

    Dim date1, date2 As Date   'variables for test
    testName = "Test TimeValue function"
    On Error GoTo errorHandler

    date2 = "16:35:17"
    date1 = TimeValue("4:35:17 PM")
    TestUtilModule.AssertTrue(date1 = date2, "the return TimeValue is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

