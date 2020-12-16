Rem Attribute VBA_ModuleType=VBAModule
Option VBASupport 1
Option Explicit

Type MyType
    ax(3) As Integer
    bx As Double
End Type

Function doUnitTest() As String
verify_testARRAY
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testARRAY()

    TestUtilModule.TestInit

    Dim testName As String
    Dim a, b, C As Variant
    a = Array(10, 20, 30)
    testName = "Test ARRAY function"
    On Error GoTo errorHandler

    b = 10
    C = a(0)
    TestUtilModule.AssertTrue(b = C, "the return ARRAY is: " & C)

    b = 20
    C = a(1)
    TestUtilModule.AssertTrue(b = C, "the return ARRAY is: " & C)

    b = 30
    C = a(2)
    TestUtilModule.AssertTrue(b = C, "the return ARRAY is: " & C)

    Dim MyWeek, MyDay
    MyWeek = Array("Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun")

    b = "Tue"
    MyDay = MyWeek(1)   ' MyDay contains "Tue".
    TestUtilModule.AssertTrue(b = MyDay, "the return ARRAY is: " & MyDay)

    b = "Thu"
        MyDay = MyWeek(3)   ' MyDay contains "Thu".
    TestUtilModule.AssertTrue(b = MyDay, "the return ARRAY is: " & MyDay)

Dim mt As MyType
    mt.ax(0) = 42
    mt.ax(1) = 43
    mt.bx = 3.14
     b = 43
        C = mt.ax(1)
    TestUtilModule.AssertTrue(b = C, "the return ARRAY is: " & C)

    b = 3.14
    C = mt.bx
    TestUtilModule.AssertTrue(b = C, "the return ARRAY is: " & C)

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

