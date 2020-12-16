Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testIsNumeric
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testIsNumeric()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test IsNumeric function"
    On Error GoTo errorHandler

    date2 = True
    date1 = IsNumeric(123)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsNumeric is: " & date1)

    date2 = True
    date1 = IsNumeric(-123)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsNumeric is: " & date1)

    date2 = True
    date1 = IsNumeric(123.8)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsNumeric is: " & date1)

    date2 = False
    date1 = IsNumeric("a")
    TestUtilModule.AssertTrue(date1 = date2, "the return IsNumeric is: " & date1)

rem    date2 = True
rem    date1 = IsNumeric(True)
rem    TestUtilModule.AssertTrue(date1 = date2, "the return IsNumeric is: " & date1)

    date2 = True
    date1 = IsNumeric("123")
    TestUtilModule.AssertTrue(date1 = date2, "the return IsNumeric is: " & date1)

    date2 = True
    date1 = IsNumeric("+123")
    TestUtilModule.AssertTrue(date1 = date2, "the return IsNumeric is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

