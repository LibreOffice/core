Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testQBcolor
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testQBcolor()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2 As Long
    testName = "Test QBcolor function"
    On Error GoTo errorHandler

    date2 = 0
    date1 = QBColor(0)
    TestUtilModule.AssertTrue(date1 = date2, "the return QBcolor is: " & date1)

    date2 = 8388608
    date1 = QBColor(1)
    TestUtilModule.AssertTrue(date1 = date2, "the return QBcolor is: " & date1)

    date2 = 32768
    date1 = QBColor(2)
    TestUtilModule.AssertTrue(date1 = date2, "the return QBcolor is: " & date1)

    date2 = 8421376
    date1 = QBColor(3)
    TestUtilModule.AssertTrue(date1 = date2, "the return QBcolor is: " & date1)

    date2 = 128
    date1 = QBColor(4)
    TestUtilModule.AssertTrue(date1 = date2, "the return QBcolor is: " & date1)

    date2 = 8388736
    date1 = QBColor(5)
    TestUtilModule.AssertTrue(date1 = date2, "the return QBcolor is: " & date1)

    date2 = 32896
    date1 = QBColor(6)
    TestUtilModule.AssertTrue(date1 = date2, "the return QBcolor is: " & date1)

    date2 = 12632256
    date1 = QBColor(7)
    TestUtilModule.AssertTrue(date1 = date2, "the return QBcolor is: " & date1)

    date2 = 8421504
    date1 = QBColor(8)
    TestUtilModule.AssertTrue(date1 = date2, "the return QBcolor is: " & date1)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

