Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testError
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testError()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Error function"
    On Error GoTo errorHandler

    date2 = Error(11)   'https://help.libreoffice.org/Basic/Error_Sub_Runtime
    date1 = "Division by zero."
    TestUtilModule.AssertTrue(date1 = date2, "the return Error is: " & date2)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

