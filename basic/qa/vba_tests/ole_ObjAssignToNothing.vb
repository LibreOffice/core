Option VBASupport 1
Function doUnitTest( TestData as String) as String
Rem Ensure object assignment is by reference
Rem when object member is used ( as lhs )
Rem This time we are testing assigning with special Nothing
Rem keyword
Set cn = New ADODB.Connection
Dim conStr As String
conStr = "Provider=MSDASQL;Driver={Microsoft Excel Driver (*.xls)};DBQ="
conStr = conStr & TestData & "; ReadOnly=False;"
cn.Open conStr
Set objCmd = New ADODB.Command
objCmd.ActiveConnection = Nothing
if objCmd.ActiveConnection Is Nothing Then
    doUnitTest = "OK" ' no error
Else
    doUnitTest = "Fail - expected objCmd.ActiveConnection be Nothing"
End If
End Function
