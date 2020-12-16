Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testCSng
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testCSng()

    TestUtilModule.TestInit

    Dim testName As String
    Dim nr1, nr2 As Single   'variables for test
    Dim nr3 As Double

    testName = "Test CSng function"
    On Error GoTo errorHandler

    nr2 = 8.534535408
    nr1 = CSng(8.534535408)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CSng is: " & nr1)

    nr3 = 100.1234
    nr2 = 100.1234
    nr1 = CSng(nr3)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CSng is: " & nr1)

    nr2 = 0
    nr1 = CSng(0)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CSng is: " & nr1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

