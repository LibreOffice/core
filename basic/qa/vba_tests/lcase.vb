Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testLCase
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testLCase()

    TestUtilModule.TestInit

    Dim testName As String
    Dim str1, str2 As String 'variables for test
    testName = "Test LCase function"
    On Error GoTo errorHandler

    str2 = "lowercase"
    str1 = LCase("LOWERCASE")
    TestUtilModule.AssertTrue(str1 = str2, "the return LCase is: " & str1)

    str2 = "lowercase"
    str1 = LCase("LowerCase")
    TestUtilModule.AssertTrue(str1 = str2, "the return LCase is: " & str1)

    str2 = "lowercase"
    str1 = LCase("lowercase")
    TestUtilModule.AssertTrue(str1 = str2, "the return LCase is: " & str1)

    str2 = "lower case"
    str1 = LCase("LOWER CASE")
    TestUtilModule.AssertTrue(str1 = str2, "the return LCase is: " & str1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

