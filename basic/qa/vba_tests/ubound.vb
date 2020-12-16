Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testUBound
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testUBound()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test UBound function"
    On Error GoTo errorHandler
    Dim A(1 To 100, 0 To 3, -3 To 4)

    date2 = 100
    date1 = UBound(A, 1)
    TestUtilModule.AssertTrue(date1 = date2, "the return UBound is: " & date1)

    date2 = 3
    date1 = UBound(A, 2)
    TestUtilModule.AssertTrue(date1 = date2, "the return UBound is: " & date1)

    date2 = 4
    date1 = UBound(A, 3)
    TestUtilModule.AssertTrue(date1 = date2, "the return UBound is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

