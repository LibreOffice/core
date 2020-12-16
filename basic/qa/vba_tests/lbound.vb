Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testLBound
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testLBound()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    Dim MyArray(1 To 10, 5 To 15, 10 To 20)     ' Declare array variables.
    testName = "Test LBound function"
    On Error GoTo errorHandler

    date2 = 1
    date1 = LBound(MyArray, 1)
    TestUtilModule.AssertTrue(date1 = date2, "the return LBound is: " & date1)

    date2 = 10
    date1 = LBound(MyArray, 3)
    TestUtilModule.AssertTrue(date1 = date2, "the return LBound is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

