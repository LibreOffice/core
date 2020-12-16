Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testMid
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testMid()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Mid function"
    On Error GoTo errorHandler

    date2 = "Mid"
    date1 = Mid("Mid Function Demo", 1, 3)
    TestUtilModule.AssertEqual(date1, date2, "the return Mid is: " & date1)

    date2 = "Demo"
    date1 = Mid("Mid Function Demo", 14, 4)
    TestUtilModule.AssertEqual(date1, date2, "the return Mid is: " & date1)

    date2 = "Function Demo"
    date1 = Mid("Mid Function Demo", 5)
    TestUtilModule.AssertEqual(date1, date2, "the return Mid is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

