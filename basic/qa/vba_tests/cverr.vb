Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testCVErr
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testCVErr()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test CVErr function"
    On Error GoTo errorHandler

    date2 = "Error 3001"
    date1 = CStr(CVErr(3001))
    TestUtilModule.AssertTrue(date1 = date2, "the return CVErr is: " & date1)

    date2 = "Error 2007"
    date1 = CStr(CVErr(xlErrDiv0))
    TestUtilModule.AssertTrue(date1 = date2, "the return CVErr is: " & date1)

    date2 = "Error 2042"
    date1 = CStr(CVErr(xlErrNA))
    TestUtilModule.AssertTrue(date1 = date2, "the return CVErr is: " & date1)

    date2 = "Error 2029"
    date1 = CStr(CVErr(xlErrName))
    TestUtilModule.AssertTrue(date1 = date2, "the return CVErr is: " & date1)

    date2 = "Error 2000"
    date1 = CStr(CVErr(xlErrNull))
    TestUtilModule.AssertTrue(date1 = date2, "the return CVErr is: " & date1)

    date2 = "Error 2036"
    date1 = CStr(CVErr(xlErrNum))
    TestUtilModule.AssertTrue(date1 = date2, "the return CVErr is: " & date1)

    date2 = "Error 2023"
    date1 = CStr(CVErr(xlErrRef))
    TestUtilModule.AssertTrue(date1 = date2, "the return CVErr is: " & date1)

    date2 = "Error 2015"
    date1 = CStr(CVErr(xlErrValue))
    TestUtilModule.AssertTrue(date1 = date2, "the return CVErr is: " & date1)

    ' tdf#79426 - passing an error object to a function
    TestUtilModule.AssertTrue(TestCVErr( CVErr( 2 ) ) = 2)
    ' tdf#79426 - test with Error-Code 448 ( ERRCODE_BASIC_NAMED_NOT_FOUND )
    TestUtilModule.AssertTrue(TestCVErr( CVErr( 448 ) ) = 448)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

Function TestCVErr(vErr As Variant)
    Dim nValue As Integer
    nValue = vErr
    TestCVErr = nValue
End Function

