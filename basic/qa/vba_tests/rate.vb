Option VBASupport 1
Rem Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testRATE
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testRATE()

    TestUtilModule.TestInit

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim date1, date2
    testName = "Test RATE function"
    On Error GoTo errorHandler

    date2 = 0.07
    date1 = Rate(3, -5, 0, 16)
    TestUtilModule.AssertTrue(Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1)

    date2 = 0
    date1 = Rate(3, -5, 0, 15)
    TestUtilModule.AssertTrue(Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1)
    
    date2 = 0.79
    date1 = Rate(3, -5, 0, 30)
    TestUtilModule.AssertTrue(Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1)

    date2 = 1
    date1 = Rate(3, -5, 0, 35)
    TestUtilModule.AssertTrue(Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1)

    date2 = 0.077
    date1 = Rate(4, -300, 1000, 0, 0)
    TestUtilModule.AssertTrue(Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1)

    date2 = 0.14
    date1 = Rate(4, -300, 1000, 0, 1)
    TestUtilModule.AssertTrue(Round(date1, 2) = Round(date2, 2), "the return RATE is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

