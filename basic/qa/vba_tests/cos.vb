Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testCOS
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testCOS()

    TestUtilModule.TestInit

    Dim testName As String
    Dim nr1, nr2 As Double   'variables for test
    testName = "Test COS function"

    On Error GoTo errorHandler

    nr2 = -0.532833020333398
    nr1 = Cos(23)
    TestUtilModule.AssertTrue(Round(nr1, 14) = Round(nr2, 14), "the return COS is: " & nr1)

    nr2 = 0.980066577841242
    nr1 = Cos(0.2)
    TestUtilModule.AssertTrue(Round(nr1, 14) = Round(nr2, 14), "the return COS is: " & nr1)

    nr2 = 0.487187675007006
    nr1 = Cos(200)
    TestUtilModule.AssertTrue(Round(nr1, 14) = Round(nr2, 14), "the return COS is: " & nr1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

