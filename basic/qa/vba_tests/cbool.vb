Option VBASupport 1
Rem Option VBASupport 1    'unREM in .vb file
Option Explicit

Function doUnitTest() As String
verify_testCBool
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testCBool()

    TestUtilModule.TestInit

    Dim testName As String
    Dim res2, res1 As Boolean
    Dim a1, a2 As Integer
    testName = "Test CBool function"
    On Error GoTo errorHandler

    res2 = True
    res1 = CBool(1)
    TestUtilModule.AssertTrue(res1 = res2, "the return CBool is: " & res1)

    res2 = False
    res1 = CBool(1 = 2)
    TestUtilModule.AssertTrue(res1 = res2, "the return CBool is: " & res1)

    res2 = False
    res1 = CBool(0)
    TestUtilModule.AssertTrue(res1 = res2, "the return CBool is: " & res1)

    res2 = True
    res1 = CBool(21)
    TestUtilModule.AssertTrue(res1 = res2, "the return CBool is: " & res1)

    res2 = True
    res1 = CBool("true")
    TestUtilModule.AssertTrue(res1 = res2, "the return CBool is: " & res1)

    res2 = False
    res1 = CBool("false")
    TestUtilModule.AssertTrue(res1 = res2, "the return CBool is: " & res1)

    res2 = True
    res1 = CBool("1")
    TestUtilModule.AssertTrue(res1 = res2, "the return CBool is: " & res1)

    res2 = True
    res1 = CBool("-1")
    TestUtilModule.AssertTrue(res1 = res2, "the return CBool is: " & res1)

    res2 = False
    res1 = CBool("0")
    TestUtilModule.AssertTrue(res1 = res2, "the return CBool is: " & res1)

    res2 = False
    a1 = 1: a2 = 10
    res1 = CBool(a1 = a2)
    TestUtilModule.AssertTrue(res1 = res2, "the return CBool is: " & res1)

    res2 = True
    a1 = 10: a2 = 10
    res1 = CBool(a1 = a2)
    TestUtilModule.AssertTrue(res1 = res2, "the return CBool is: " & res1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

