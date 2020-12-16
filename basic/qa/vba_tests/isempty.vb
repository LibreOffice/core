Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testIsEmpty
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testIsEmpty()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2, MyVar
    testName = "Test IsEmpty function"
    On Error GoTo errorHandler

    date2 = True
    date1 = IsEmpty(MyVar)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsEmpty is: " & date1)

    MyVar = Null    ' Assign Null.
    date2 = False
    date1 = IsEmpty(MyVar)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsEmpty is: " & date1)

    MyVar = Empty    ' Assign Empty.
    date2 = True
    date1 = IsEmpty(MyVar)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsEmpty is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

