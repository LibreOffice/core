Option VBASupport 1
Option Explicit

Rem Test accessing an object that has default object member
Rem which in turn has a default member that is a method
Function doUnitTest(TestData As String) As String
doUnitTest = "Begin"
Dim modifiedTimout As Long
Dim cnn1 As New ADODB.Connection
Dim rst1 As New ADODB.Recordset
Dim conStr As String
cnn1.Open "Provider=Microsoft.Jet.OLEDB.4.0;" & _
"Data Source=" & TestData & ";" & _
"Extended Properties=""Excel 8.0;HDR=Yes"";"
rst1.Open "SELECT * FROM [Sheet1$];", cnn1, adOpenStatic, adLockReadOnly
Dim val
val = rst1("FirstName")
If val = "Paddy" Then
    doUnitTest = "OK"
Else
    doUnitTest = "Failed, expected 'Paddy' got " & val
End If

End Function
