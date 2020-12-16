Rem Attribute VBA_ModuleType=VBAModule
Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testASC
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testASC()

    TestUtilModule.TestInit

    Dim testName As String
    Dim nr1, nr2 As Double

    testName = "Test ASC function"
    On Error GoTo errorHandler

    nr2 = 65
    nr1 = Asc("A")
    TestUtilModule.AssertTrue(nr1 = nr2, "the return ASC is: " & nr1)

    nr2 = 97
    nr1 = Asc("a")
    TestUtilModule.AssertTrue(nr1 = nr2, "the return ASC is: " & nr1)

    nr2 = 65
    nr1 = Asc("Apple")
    TestUtilModule.AssertTrue(nr1 = nr2, "the return ASC is: " & nr1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

