Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testPartition
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testPartition()

    TestUtilModule.TestInit

    Dim testName As String
    Dim retStr As String
    testName = "Test Partition function"
    On Error GoTo errorHandler

    retStr = Partition(20, 0, 98, 5)
    'MsgBox retStr
    TestUtilModule.AssertTrue(retStr = "20:24", "the number 20 occurs in the range:" & retStr)

    retStr = Partition(20, 0, 99, 1)
    'MsgBox retStr
    TestUtilModule.AssertTrue(retStr = " 20: 20", "the number 20 occurs in the range:" & retStr)

    retStr = Partition(120, 0, 99, 5)
    'MsgBox retStr
    TestUtilModule.AssertTrue(retStr = "100:   ", "the number 120 occurs in the range:" & retStr)

     retStr = Partition(-5, 0, 99, 5)
    'MsgBox retStr
    TestUtilModule.AssertTrue(retStr = "   : -1", "the number -5 occurs in the range:" & retStr)

     retStr = Partition(2, 0, 5, 2)
    'MsgBox retStr
    TestUtilModule.AssertTrue(retStr = " 2: 3", "the number 2 occurs in the range:" & retStr)
    TestUtilModule.TestEnd
    Exit Sub
errorHandler:
    TestUtilModule.AssertTrue(False, "verify_testPartition failed, hit error handler")
End Sub

