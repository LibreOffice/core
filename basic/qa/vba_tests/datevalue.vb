Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testDateValue
doUnitTest = TestUtilModule.GetResult()
End Function

Function verify_testDateValue() as String

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2 As Date
    testName = "Test DateValue function"
    date2 = 25246

    On Error GoTo errorHandler

    date1 = DateValue("February 12, 1969") '2/12/1969
    TestUtilModule.AssertTrue(date1 = date2, "the return date is: " & date1)

    date2 = 39468
    date1 = DateValue("21/01/2008") '1/21/2008
    TestUtilModule.AssertTrue(date1 = date2, "the return date is: " & date1)
    TestUtilModule.TestEnd

    Exit Function
errorHandler:
    TestUtilModule.AssertTrue(False, "ERROR", "#" & Str(Err.Number) &" at line"& Str(Erl) &" - "& Error$)
    TestUtilModule.TestEnd
End Sub

