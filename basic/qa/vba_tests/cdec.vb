Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testCDec
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testCDec()

    TestUtilModule.TestInit

    Dim testName As String
    Dim ret As Double
    testName = "Test CDec function"
    On Error GoTo errorHandler

    ret = CDec("")
    TestUtilModule.AssertTrue(ret = 0, "Empty string test:" & ret)

    ret = CDec("1234")
    TestUtilModule.AssertTrue(ret = 1234, "Simple number:" & ret)

    ret = CDec("  1234  ")
    TestUtilModule.AssertTrue(ret = 1234, "Simple number with whitespaces:" & ret)

    ret = CDec("-1234")
    TestUtilModule.AssertTrue(ret = -1234, "Simple negative number:" & ret)

    ret = CDec(" - 1234 ")
    TestUtilModule.AssertTrue(ret = -1234, "Simple negative number with whitespaces:" & ret)

    '''''''''''''''
    ' Those are erroneous, see i#64348
    ret = CDec("1234-")
    TestUtilModule.AssertTrue(ret = -1234, "Wrong negative number1:" & ret)

    ret = CDec("  1234  -")
    TestUtilModule.AssertTrue(ret = -1234, "Wrong negative number2:" & ret)

    'ret = CDec("79228162514264300000000000001")
    'TestUtilModule.AssertTrue(ret = 79228162514264300000000000001, "Very long number1:" & ret)
    'ret = ret+1
    'TestUtilModule.AssertTrue(ret = 79228162514264300000000000002, "Very long number2:" & ret)

    ret = CDec("79228162514264400000000000000")
    TestUtilModule.AssertTrue(ret = 62406456049664, "Very long number3:" & ret)

    ret = CDec("79228162514264340000000000000") ' gives zero
    TestUtilModule.AssertTrue(ret = 0, "Very long number4:" & ret)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

