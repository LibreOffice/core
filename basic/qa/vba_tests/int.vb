Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testInt
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testInt()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Int function"
    On Error GoTo errorHandler

    date2 = 99
    date1 = Int(99.8)
    TestUtilModule.AssertTrue(date1 = date2, "the return Int is: " & date1)

    date2 = -100
    date1 = Int(-99.8)
    TestUtilModule.AssertTrue(date1 = date2, "the return Int is: " & date1)

    date2 = -100
    date1 = Int(-99.2)
    TestUtilModule.AssertTrue(date1 = date2, "the return Int is: " & date1)

    date2 = 0
    date1 = Int(0.2)
    TestUtilModule.AssertTrue(date1 = date2, "the return Int is: " & date1)

    date2 = 0
    date1 = Int(0)
    TestUtilModule.AssertTrue(date1 = date2, "the return Int is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

