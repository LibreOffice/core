Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testStrReverse
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testStrReverse()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test StrReverse function"
    On Error GoTo errorHandler

    date2 = "dcba"
    date1 = StrReverse("abcd")
    TestUtilModule.AssertTrue(date1 = date2, "the return StrReverse is: " & date1)

    date2 = "BABABA"
    date1 = StrReverse("ABABAB")
    TestUtilModule.AssertTrue(date1 = date2, "the return StrReverse is: " & date1)

    date2 = "654321"
    date1 = StrReverse("123456")
    TestUtilModule.AssertTrue(date1 = date2, "the return StrReverse is: " & date1)

    date2 = "6"
    date1 = StrReverse(6)
    TestUtilModule.AssertTrue(date1 = date2, "the return StrReverse is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

