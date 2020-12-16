Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testSTRcomp
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testSTRcomp()

    TestUtilModule.TestInit

    Dim testName As String
    Dim TestStr, TestStr1, TestStr2 As String
    Dim date1, date2
    testName = "Test STRcomp function"
    On Error GoTo errorHandler
    TestStr1 = "ABCD"
    TestStr2 = "abcd"

    date2 = 0
    date1 = StrComp(TestStr1, TestStr2, vbTextCompare)
    TestUtilModule.AssertTrue(date1 = date2, "the return STRcomp is: " & date1)

    date2 = -1
    date1 = StrComp(TestStr1, TestStr2, vbBinaryCompare)
    TestUtilModule.AssertTrue(date1 = date2, "the return STRcomp is: " & date1)

    date2 = -1
    date1 = StrComp(TestStr1, TestStr2)
    TestUtilModule.AssertTrue(date1 = date2, "the return STRcomp is: " & date1)

    date2 = 0
    date1 = StrComp("text", "text", vbBinaryCompare)
    TestUtilModule.AssertTrue(date1 = date2, "the return STRcomp is: " & date1)

    date2 = 1
    date1 = StrComp("text  ", "text", vbBinaryCompare)
    TestUtilModule.AssertTrue(date1 = date2, "the return STRcomp is: " & date1)

    date2 = -1
    date1 = StrComp("Text", "text", vbBinaryCompare)
    TestUtilModule.AssertTrue(date1 = date2, "the return STRcomp is: " & date1)

    date2 = 0
    date1 = StrComp("text", "text", vbTextCompare)
    TestUtilModule.AssertTrue(date1 = date2, "the return STRcomp is: " & date1)

    date2 = 1
    date1 = StrComp("text  ", "text", vbTextCompare)
    TestUtilModule.AssertTrue(date1 = date2, "the return STRcomp is: " & date1)

    date2 = 0
    date1 = StrComp("Text", "text", vbTextCompare)
    TestUtilModule.AssertTrue(date1 = date2, "the return STRcomp is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

