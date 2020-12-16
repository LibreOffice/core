Option VBASupport 1
Option Explicit

Function doUnitTest() As String
    verify_ByteArrayString
    doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_ByteArrayString()
    Dim testName As String
    Dim MyString As String
    Dim x() As Byte
    Dim count As Integer
    testName = "Test the conversion between bytearray and string"

    On Error GoTo errorHandler

    MyString = "abc"
    x = MyString ' string -> byte array

    TestUtilModule.TestInit

    count = UBound(x) ' 6 byte

    ' test bytes in string
    TestUtilModule.AssertEqual((count), 5, "test1 numbytes ")

    MyString = x 'byte array -> string
    TestUtilModule.AssertEqual(MyString, "abc", "test assign byte array to string")

    TestUtilModule.TestEnd
    Exit Sub
errorHandler:
    TestUtilModule.AssertTrue(False, "ERROR", "#"& Str(Err.Number) &" at line"& Str(Erl) &" - "& Error$)
    TestUtilModule.TestEnd
End Sub
