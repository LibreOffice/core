Rem Attribute VBA_ModuleType=VBAModule
Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testABS
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testABS()

    TestUtilModule.TestInit

    Dim testName As String
    Dim nr1, nr2 As Double

    testName = "Test ABS function"
    On Error GoTo errorHandler

    nr2 = 5
    nr1 = Abs(-5)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return ABS is: " & nr1)

    nr2 = 5
    nr1 = Abs(5)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return ABS is: " & nr1)

    nr2 = 21.7
    nr1 = Abs(-21.7)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return ABS is: " & nr1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

