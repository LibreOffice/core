Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testCInt
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testCInt()

    TestUtilModule.TestInit

    Dim testName As String
    Dim nr1, nr2 As Integer   'variables for test
    testName = "Test CInt function"

    On Error GoTo errorHandler

    nr2 = -1
    nr1 = CInt(-1.1)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CInt is: " & nr1)

    nr2 = -1
    nr1 = CInt(-1.1)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CInt is: " & nr1)

    nr2 = -2
    nr1 = CInt(-1.9)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CInt is: " & nr1)

    nr2 = 0
    nr1 = CInt(0.2)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CInt is: " & nr1)

REM In excel:
REM    If the fraction is less than or equal to .5, the result will round down.
REM    If the fraction is greater than .5, the result will round up.

REM    nr2 = 0
REM    nr1 = CInt(0.5)
REM    TestUtilModule.AssertTrue(nr1 = nr2, "the return CInt is: " & nr1)

REM    nr2 = 2
REM    nr1 = CInt(1.5)
REM    TestUtilModule.AssertTrue(nr1 = nr2, "the return CInt is: " & nr1)

REM    nr2 = 2
REM    nr1 = CInt(2.5)
REM    TestUtilModule.AssertTrue(nr1 = nr2, "the return CInt is: " & nr1)

    nr2 = 11
    nr1 = CInt(10.51)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CInt is: " & nr1)

    nr2 = 30207
    nr1 = CInt("&H75FF")
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CInt is: " & nr1)

    nr2 = 1876
    nr1 = CInt("&H754")
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CInt is: " & nr1)

    nr2 = 21
    nr1 = CInt("+21")
    TestUtilModule.AssertTrue(nr1 = nr2, "the return CInt is:" & nr1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

