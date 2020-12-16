Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testATN
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testATN()

    TestUtilModule.TestInit

    Dim testName As String
    Dim nr1, nr2 As Double
    testName = "Test ATN function"
    On Error GoTo errorHandler

    nr2 = 1.10714871779409
    nr1 = Atn(2)
    TestUtilModule.AssertTrue(Round(nr1, 14) = Round(nr2, 14), "the return ATN is: " & nr1)

    nr2 = 1.19166451926354
    nr1 = Atn(2.51)
    TestUtilModule.AssertTrue(Round(nr1, 14) = Round(nr2, 14), "the return ATN is: " & nr1)

    nr2 = -1.27229739520872
    nr1 = Atn(-3.25)
    TestUtilModule.AssertTrue(Round(nr1, 14) = Round(nr2, 14), "the return ATN is: " & nr1)

    nr2 = 1.56603445802574
    nr1 = Atn(210)
    TestUtilModule.AssertTrue(Round(nr1, 14) = Round(nr2, 14), "the return ATN is: " & nr1)

    nr2 = 0
    nr1 = Atn(0)
    TestUtilModule.AssertTrue(nr1 = nr2, "the return ATN is: " & nr1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

