Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testSpace
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testSpace()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Space function"
    On Error GoTo errorHandler

    date2 = "  "
    date1 = Space(2)
    TestUtilModule.AssertTrue(date1 = date2, "the return Space is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

