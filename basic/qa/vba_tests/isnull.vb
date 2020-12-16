Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testIsNull
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testIsNull()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test IsNull function"
    On Error GoTo errorHandler

    date2 = True
    date1 = IsNull(Null)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsNull is: " & date1)

    date2 = False
    date1 = IsNull("")
    TestUtilModule.AssertTrue(date1 = date2, "the return IsNull is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

