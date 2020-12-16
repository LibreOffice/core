Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testIIf
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testIIf()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2, testnr
    testName = "Test IIf function"
    On Error GoTo errorHandler

    date2 = "it is true"
    date1 = IIf(True, "it is true", "it is false")
    TestUtilModule.AssertTrue(date1 = date2, "the return IIf is: " & date1)

    date2 = "it is false"
    date1 = IIf(False, "It is true", "it is false")
    TestUtilModule.AssertTrue(date1 = date2, "the return IIf is: " & date1)

    testnr = 1001
    date2 = "Large"
    date1 = IIf(testnr > 1000, "Large", "Small")
    TestUtilModule.AssertTrue(date1 = date2, "the return IIf is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

