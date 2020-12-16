Option VBASupport 1
Rem Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testRGB
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testRGB()

    TestUtilModule.TestInit

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim date1, date2
    testName = "Test RGB function"
    On Error GoTo errorHandler

    date2 = 255
    date1 = RGB(255, 0, 0)
    TestUtilModule.AssertTrue(date1 = date2, "the return RGB is: " & date1)

    date2 = 13339467
    date1 = RGB(75, 139, 203)
    TestUtilModule.AssertTrue(date1 = date2, "the return RGB is: " & date1)

    date2 = 16777215
    date1 = RGB(255, 255, 255)
    TestUtilModule.AssertTrue(date1 = date2, "the return RGB is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

