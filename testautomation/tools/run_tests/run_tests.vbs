'*************************************************************************
'*
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' 
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
WScript.Echo "****************************************************"
WScript.Echo "************ STARTING ************"
WScript.Echo "****************************************************"

'echo "My name is: " $USER

' test if location exists
If oFSO.FolderExists(sLocation) Then
	WScript.Echo "using scripts from " & sLocation
Else
	WScript.Echo "test scripts not found at " & sLocation
	WScript.Echo "Please set sLocation in this script"
	WScript.Quit 1
End If

' test if testtool.bin exists
If oFSO.FileExists(sTestTool) Then
	WScript.Echo "using testtool from " & sTestTool
Else
	WScript.Echo "testtool not found at " & sTestTool
	WScript.Echo "Please set sTestTool in this script"
	WScript.Quit 1
End If

i = 0
'--- Main loop
While Not oStdIn.AtEndOfStream

    WScript.echo "Running soffices' processes: "
    '# kill office, if exists
    '`$KILLOFFICEALL`
    ' *************-> have this line modified, because an error in 
    ' *************-> testtool.exe. (the soffice processes are not
    ' *************-> killed in resetoffice.bas) 
    ' *************-> 2009/07/06
    ' *************-> wolfgang pechlaner (wope@openoffice.org)       
    WshShell.Run "tskill soffice", 1, true
    WScript.Sleep 1000 

    sTestCase = oStdIn.ReadLine

    WScript.echo " "
    WScript.Echo "****************************************************"
    WScript.Echo "running " & sTestCase
    sTest = sLocation & sTestCase
    WScript.echo "*** -> " & sTest

    If not oFSO.FileExists( sTest ) Then
        WScript.Echo " Could not run " & sTest 
        WScript.Echo " File not found"
    Else
        ' first run is the real test ...
        Set oExec = WshShell.Exec("""" & sTestTool & """ & -run & """ & sTest & """" )
        WScript.Sleep 1000
        
        If oExec.Status = 0 Then
            WScript.Echo " Successfully started"
        Else
            WScript.Echo " There might be something wrong with starting the Testtool!"
        End If

        '--- wait until Testtool has finished & closed
        While oExec.Status = 0
            WScript.Sleep 1000
            i = i+1
        Wend
    End If
    'run the office exit script!
    WScript.Echo " trying to kill the previous office process"
    WshShell.Run """" & sTestTool & """ -run """ & sExitOfficeBas & """", 1, true 

    Set oExec = Nothing
Wend

WScript.Echo "Duration:" & i & "sec " 
