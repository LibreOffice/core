Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testIsArray
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testIsArray()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    Dim MyArray(1 To 5) As Integer, YourArray    ' Declare array variables.
    testName = "Test IsArray function"
    On Error GoTo errorHandler
    YourArray = Array(1, 2, 3)    ' Use Array function.

    date2 = True
    date1 = IsArray(MyArray)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsArray is: " & date1)

    date2 = True
    date1 = IsArray(YourArray)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsArray is: " & date1)

    date2 = False
    date1 = IsArray(date2)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsArray is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

