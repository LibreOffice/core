Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testInStrRev
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testInStrRev()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2, SearchString, SearchChar
    testName = "Test InStrRev function"
    On Error GoTo errorHandler

    date2 = 5
    date1 = InStrRev("somemoretext", "more", -1)
    TestUtilModule.AssertTrue(date1 = date2, "the return InStrRev is: " & date1)

    date2 = 5
    date1 = InStrRev("somemoretext", "more")
    TestUtilModule.AssertTrue(date1 = date2, "the return InStrRev is: " & date1)

    date2 = 1
    date1 = InStrRev("somemoretext", "somemoretext")
    TestUtilModule.AssertTrue(date1 = date2, "the return InStrRev is: " & date1)

    date2 = 0
    date1 = InStrRev("somemoretext", "nothing")
    TestUtilModule.AssertTrue(date1 = date2, "the return InStrRev is: " & date1)

    SearchString = "XXpXXpXXPXXP"   ' String to search in.
    SearchChar = "P"    ' Search for "P".
    date2 = 3
    date1 = InStrRev(SearchString, SearchChar, 4, 1)
    TestUtilModule.AssertTrue(date1 = date2, "the return InStrRev is: " & date1)

    date2 = 12
    date1 = InStrRev(SearchString, SearchChar, -1, 0)
    TestUtilModule.AssertTrue(date1 = date2, "the return InStrRev is: " & date1)
    
    date2 = 0
    date1 = InStrRev(SearchString, "W", 1)
    TestUtilModule.AssertTrue(date1 = date2, "the return InStrRev is: " & date1)
    
    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

