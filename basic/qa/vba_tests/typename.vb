Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testTypeName
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testTypeName()

    TestUtilModule.TestInit

    Dim testName As String
    Dim date1, date2
    testName = "Test TypeName function"
    On Error GoTo errorHandler
    Dim b1 As Boolean
    Dim c1 As Byte
    Dim d1 As Date
    Dim d2 As Double
    Dim i1 As Integer
    Dim l1 As Long

    date2 = "String"
    date1 = TypeName(testName)
    TestUtilModule.AssertTrue(date1 = date2, "the return TypeName is: " & date1)

    date2 = "Boolean"
    date1 = TypeName(b1)
    TestUtilModule.AssertTrue(date1 = date2, "the return TypeName is: " & date1)

    date2 = "Byte"
    date1 = TypeName(c1)
    TestUtilModule.AssertTrue(date1 = date2, "the return TypeName is: " & date1)

    date2 = "Date"
    date1 = TypeName(d1)
    TestUtilModule.AssertTrue(date1 = date2, "the return TypeName is: " & date1)

    date2 = "Double"
    date1 = TypeName(d2)
    TestUtilModule.AssertTrue(date1 = date2, "the return TypeName is: " & date1)

    date2 = "Integer"
    date1 = TypeName(i1)
    TestUtilModule.AssertTrue(date1 = date2, "the return TypeName is: " & date1)

    date2 = "Long"
    date1 = TypeName(l1)
    TestUtilModule.AssertTrue(date1 = date2, "the return TypeName is: " & date1)

    ' tdf#129596 - Types of constant values
    TestUtilModule.AssertTrue(TypeName(32767) = "Integer", "the return TypeName(32767) is: " & TypeName(32767))
    TestUtilModule.AssertTrue(TypeName(-32767) = "Integer", "the return TypeName(-32767) is: " & TypeName(-32767))
    TestUtilModule.AssertTrue(TypeName(1048575) = "Long", "the return TypeName(1048575) is: " & TypeName(1048575))
    TestUtilModule.AssertTrue(TypeName(-1048575) = "Long", "the return TypeName(-1048575) is: " & TypeName(-1048575))

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

