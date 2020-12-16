Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testVarType
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testVarType()

    TestUtilModule.TestInit

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim TestInt As Integer
    Dim TestLong As Long
    Dim TestDouble As Double
    Dim TestBoo As Boolean
    Dim date1, date2
    testName = "Test VarType function"
    On Error GoTo errorHandler

    date2 = 8
    date1 = VarType(testName)
    TestUtilModule.AssertTrue(date1 = date2, "the return VarType is: " & date1)

    date2 = 11
    date1 = VarType(TestBoo)
    TestUtilModule.AssertTrue(date1 = date2, "the return VarType is: " & date1)

    date2 = 5
    date1 = VarType(TestDouble)
    TestUtilModule.AssertTrue(date1 = date2, "the return VarType is: " & date1)

    date2 = 3
    date1 = VarType(TestLong)
    TestUtilModule.AssertTrue(date1 = date2, "the return VarType is: " & date1)

    date2 = 2
    date1 = VarType(TestInt)
    TestUtilModule.AssertTrue(date1 = date2, "the return VarType is: " & date1)

    date2 = 7
    date1 = VarType(TestDateTime)
    TestUtilModule.AssertTrue(date1 = date2, "the return VarType is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

