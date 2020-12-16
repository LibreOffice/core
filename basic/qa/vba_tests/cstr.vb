Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testCStr
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testCStr()

    TestUtilModule.TestInit

    Dim testName As String
    Dim str2, str3
    Dim str1 As String   'variables for test
    testName = "Test CStr function"
    On Error GoTo errorHandler

    str3 = 437.324
    str2 = "437.324"
    str1 = CStr(str3)
    TestUtilModule.AssertTrue(str1 = str2, "the return CStr is: " & str1)

    str2 = "500"
    str1 = CStr(500)
    TestUtilModule.AssertTrue(str1 = str2, "the return CStr is: " & str1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

