Wscript.Echo "Running VBScript for detecting antivirus..." & VBNewLine 
machineName = "."
Set objWMIService = GetObject("winmgmts:{impersonationLevel=impersonate}!\\" & machineName & "\root\SecurityCenter2")
Set installedAntiviruses = objWMIService.ExecQuery("Select * from AntivirusProduct")
'iterates through all the antivirus software,retrieved by the WMI query,present on the system and prints only the ones that are active
'this is done by checking the 12th bit of the productState property of the antivirus
'if 12th bit is on then it means that the antivirus is in active state
'if 12th bit is off then it is inactive.
count=0
list=""
For Each antivirus in installedAntiviruses
    If  antivirus.productState And &h01000 Then 'checking the state of the 12th bit of productState property of the antivirus
    count=count+1
    list=list & "* " & antivirus.displayName & VBNewLine
	
     End if
Next

If count = 1 Then
 Wscript.Echo "The following Antivirus Software is active on your system and it is highly" & VBNewLine & "recommended that you exclude the build directories associated with LibreOffice:" & VBNewLine
 Wscript.Echo list

Elseif count > 1 Then
 Wscript.Echo "The following " & count & " Antivirus Software are active on your system and it is highly" & VBNewLine & "recommended that you exclude the build directories associated with LibreOffice:" & VBNewLine
 Wscript.Echo list
Else 
  Wscript.Echo  VBNewLine & "No active Antivirus Software found" & VBNewLine 
End if

