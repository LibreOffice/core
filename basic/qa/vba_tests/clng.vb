Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testCLng
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testCLng()

    TestUtilModule.TestInit

    Dim testName As String
    Dim nr1, nr2 As Long   'variables for test
    testName = "Test CLng function"

    On Error GoTo errorHandler

    nr2 = -1
    nr1 = CLng(-1.1)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CLng is: " & nr1)

    nr2 = -1
    nr1 = CLng(-1.1)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CLng is: " & nr1)

    nr2 = -2
    nr1 = CLng(-1.9)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CLng is: " & nr1)

    nr2 = 0
    nr1 = CLng(0.2)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CLng is: " & nr1)

REM    nr2 = 0
REM    nr1 = CLng(0.5)
REM    TestUtilModule.AssertTrue(nr1 = nr2, "the return CLng is: " & nr1)

REM    If the fraction is less than or equal to .5, the result will round down.
REM    If the fraction is greater than .5, the result will round up.

    nr2 = 11
    nr1 = CLng(10.51)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CLng is: " & nr1)

    nr2 = 30207
    nr1 = CLng("&H75FF")
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CLng is: " & nr1)

    nr2 = 1876
    nr1 = CLng("&H754")
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CLng is: " & nr1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

