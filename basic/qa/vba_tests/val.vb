Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testVal
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testVal()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test Val function"
    On Error GoTo errorHandler

    date2 = 2
    date1 = Val("02/04/2010")
    TestUtilModule.AssertTrue(date1 = date2, "the return Val is: " & date1)

    date2 = 1050
    date1 = Val("1050")
    TestUtilModule.AssertTrue(date1 = date2, "the return Val is: " & date1)

    date2 = 130.75
    date1 = Val("130.75")
    TestUtilModule.AssertTrue(date1 = date2, "the return Val is: " & date1)

    date2 = 50
    date1 = Val("50 Park Lane")
    TestUtilModule.AssertTrue(date1 = date2, "the return Val is: " & date1)

    date2 = 1320
    date1 = Val("1320 then some text")
    TestUtilModule.AssertTrue(date1 = date2, "the return Val is: " & date1)

    date2 = 0
    date1 = Val("L13.5")
    TestUtilModule.AssertTrue(date1 = date2, "the return Val is: " & date1)

    date2 = 0
    date1 = Val("sometext")
    TestUtilModule.AssertTrue(date1 = date2, "the return Val is: " & date1)

REM    date2 = 1
REM    date1 = Val("1, 2")
REM    TestUtilModule.AssertTrue(date1 = date2, "the return Val is: " & date1)
REM		tdf#111999

    date2 = -1
    date1 = Val("&HFFFF")
    TestUtilModule.AssertTrue(date1 = date2, "the return Val is: " & date1)
    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

