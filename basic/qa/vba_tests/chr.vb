Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testCHR
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testCHR()

    TestUtilModule.TestInit

    Dim testName As String
    Dim str1, str2 As String
    testName = "Test CHR function"
    On Error GoTo errorHandler

    str2 = "W"
    str1 = Chr(87)
    TestUtilModule.AssertTrue(str1 = str2, "the return CHR is: " & str1)

    str2 = "i"
    str1 = Chr(105)
    TestUtilModule.AssertTrue(str1 = str2, "the return CHR is: " & str1)

    str2 = "#"
    str1 = Chr(35)
    TestUtilModule.AssertTrue(str1 = str2, "the return CHR is: " & str1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

