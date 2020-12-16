Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testCdbl
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testCdbl()

    TestUtilModule.TestInit

    Dim testName As String
    Dim nr1, nr2 As Double   'variables for test
    testName = "Test Cdbl function"
    On Error GoTo errorHandler

    nr2 = 0
    nr1 = CDbl(0)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return Cdbl is: " & nr1)

    nr2 = 10.1234567890123
    nr1 = CDbl(10.1234567890123)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return Cdbl is: " & nr1)

    nr2 = 0.00005
    nr1 = CDbl(0.005 * 0.01)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return Cdbl is: " & nr1)

    nr2 = 20
    nr1 = CDbl("20")
    TestUtilModule.AssertTrue(nr1 = nr2, "the return Cdbl is: " & nr1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

