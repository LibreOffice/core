Option VBASupport 1
Option Explicit

Dim passCount As Integer
Dim failCount As Integer
Dim displayMessage As Boolean
Dim thisTest As String

Function doUnitTest() As String
Dim result As String
result = verify_ByteArrayString()
If failCount <> 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Sub Main()
MsgBox verify_ByteArrayString()
End Sub

Function verify_ByteArrayString() As String
    passCount = 0
    failCount = 0
    Dim result As String

    Dim testName As String
    Dim MyString As String
    Dim x() As Byte
    Dim count As Integer
    testName = "Test the conversion between bytearray and string"
    
    
    On Error GoTo errorHandler
    
    MyString = "abc"
    x = MyString ' string -> byte array
    
    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    count = UBound(x) ' 6 byte
    
    ' test bytes in string
    result = result + updateResultString("test1 numbytes ", (count), 5)

    
    MyString = x 'byte array -> string
    result = result + updateResultString("test assign byte array to string", MyString, "abc")
       
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_ByteArrayString = result
    Exit Function
errorHandler:
    failCount = failCount + 1
    verify_ByteArrayString = "Error Handler hit"
End Function

Function updateResultString(testDesc As String, actual As String, expected As String) As String
Dim result As String
If actual <> expected Then
    result = result & Chr$(10) & testDesc & " Failed: expected " & expected & " got " & actual
    failCount = failCount + 1
Else
    passCount = passCount + 1
End If
updateResultString = result
End Function
