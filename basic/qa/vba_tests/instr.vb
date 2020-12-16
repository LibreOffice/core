Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testInStr
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testInStr()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2, SearchString, SearchChar
    testName = "Test InStr function"
    On Error GoTo errorHandler

    date2 = 5
    date1 = InStr(1, "somemoretext", "more")
    TestUtilModule.AssertTrue(date1 = date2, "the return InStr is: " & date1)

    date2 = 5
    date1 = InStr("somemoretext", "more")
    TestUtilModule.AssertTrue(date1 = date2, "the return InStr is: " & date1)

    date2 = 1
    date1 = InStr("somemoretext", "somemoretext")
    TestUtilModule.AssertTrue(date1 = date2, "the return InStr is: " & date1)

    date2 = 0
    date1 = InStr("somemoretext", "nothing")
    TestUtilModule.AssertTrue(date1 = date2, "the return InStr is: " & date1)

    SearchString = "XXpXXpXXPXXP"   ' String to search in.
    SearchChar = "P"    ' Search for "P".
    date2 = 6
    date1 = InStr(4, SearchString, SearchChar, 1)
    TestUtilModule.AssertTrue(date1 = date2, "the return InStr is: " & date1)

    date2 = 9
    date1 = InStr(1, SearchString, SearchChar, 0)
    TestUtilModule.AssertTrue(date1 = date2, "the return InStr is: " & date1)
    
    date2 = 0
    date1 = InStr(1, SearchString, "W")
    TestUtilModule.AssertTrue(date1 = date2, "the return InStr is: " & date1)
    
    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

