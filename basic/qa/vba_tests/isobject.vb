Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testIsObject
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testIsObject()

    TestUtilModule.TestInit

    Dim testName As String
    Dim TestStr As String
    Dim MyObject As Object
    Dim date1, date2, YourObject
    testName = "Test IsObject function"
    On Error GoTo errorHandler

    Set YourObject = MyObject    ' Assign an object reference.
    date2 = True
    date1 = IsObject(YourObject)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsObject is: " & date1)

    date2 = False
    date1 = IsObject(TestStr)
    TestUtilModule.AssertTrue(date1 = date2, "the return IsObject is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

