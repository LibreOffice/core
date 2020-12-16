Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testJoin
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testJoin()

    TestUtilModule.TestInit

    Dim testName As String
    Dim str1, str2 As String
    Dim vaArray(2) As String
    testName = "Test Join function"
    On Error GoTo errorHandler
    vaArray(0) = "string1"
    vaArray(1) = "string2"
    vaArray(2) = "string3"

    str2 = "string1 string2 string3"
    str1 = Join(vaArray)
    TestUtilModule.AssertTrue(str1 = str2, "the return Join is: " & str1)

    str2 = "string1 string2 string3"
    str1 = Join(vaArray, " ")
    TestUtilModule.AssertTrue(str1 = str2, "the return Join is: " & str1)

    str2 = "string1<>string2<>string3"
    str1 = Join(vaArray, "<>")
    TestUtilModule.AssertTrue(str1 = str2, "the return Join is: " & str1)

    str2 = "string1string2string3"
    str1 = Join(vaArray, "")
    TestUtilModule.AssertTrue(str1 = str2, "the return Join is: " & str1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

