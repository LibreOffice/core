Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testHex
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testHex()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Hex function"
    On Error GoTo errorHandler

    date2 = "9"
    date1 = Hex(9)
    TestUtilModule.AssertTrue(date1 = date2, "the return Hex is: " & date1)

    date2 = "9"
    date1 = Hex(9)
    TestUtilModule.AssertTrue(date1 = date2, "the return Hex is: " & date1)

    date2 = "A"
    date1 = Hex(10)
    TestUtilModule.AssertTrue(date1 = date2, "the return Hex is: " & date1)

    date2 = "10"
    date1 = Hex(16)
    TestUtilModule.AssertTrue(date1 = date2, "the return Hex is: " & date1)

    date2 = "FF"
    date1 = Hex(255)
    TestUtilModule.AssertTrue(date1 = date2, "the return Hex is: " & date1)

    date2 = "100"
    date1 = Hex(256)
    TestUtilModule.AssertTrue(date1 = date2, "the return Hex is: " & date1)

    date2 = "1CB"
    date1 = Hex(459)
    TestUtilModule.AssertTrue(date1 = date2, "the return Hex is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

