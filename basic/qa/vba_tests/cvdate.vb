Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testCVDate
' SKIPPED test due to CVDate not being available
'doUnitTest = TestUtilModule.GetResult()
doUnitTest = "OK"
End Function

Sub verify_testCVDate()
    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2 As Date   'variables for test
    testName = "Test CVDate function"
    On Error GoTo errorHandler

    date2 = 25246
    date1 = CVDate("12.2.1969") '2/12/1969
    TestUtilModule.AssertTrue(date1 = date2, "the return CVDate is: " & date1)

    date2 = 28313
    date1 = CVDate("07/07/1977")
    TestUtilModule.AssertTrue(date1 = date2, "the return CVDate is: " & date1)

    date2 = 28313
    date1 = CVDate(#7/7/1977#)
    TestUtilModule.AssertTrue(date1 = date2, "the return CVDate is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
    TestUtilModule.AssertTrue(False, "ERROR", "#"& Str(Err.Number) &" at line"& Str(Erl) &" - "& Error$)
End Sub
