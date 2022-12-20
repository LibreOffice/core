'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Function doUnitTest(TestData as String, Driver as String) as String
Rem Ensure object assignment is by reference
Rem when object member is used ( as lhs )
Rem This time we are testing assigning with special Nothing
Rem keyword
Set cn = New ADODB.Connection
Dim conStr As String
conStr = "Provider=MSDASQL;Driver={" & Driver & "};DBQ="
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
