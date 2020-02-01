Set objWMIService = GetObject("winmgmts:{impersonationLevel=impersonate}!\\.\root\SecurityCenter2")
Set installedAntiviruses = objWMIService.ExecQuery("Select * from AntivirusProduct")
'Iterates through all the antivirus software,retrieved by the WMI query,present on the system and prints only the ones that are active
'this is done by checking the 12th bit of the productState property of the antivirus
'if 12th bit is on then it means that the antivirus is in active state
'if 12th bit is off then it is inactive.
'see http://neophob.com/2010/03/wmi-query-windows-securitycenter2/
count=0
list=""
For Each antivirus in installedAntiviruses
    If  antivirus.productState And &h01000 Then 'checking the state of the 12th bit of productState property of the antivirus
    count=count+1
    list=list & VBNewLine & VBtab & "*" & antivirus.displayName  
    End if
Next
If count = 0 Then
 Wscript.StdOut.Write "NOT_FOUND"
Else Wscript.Echo list
End if
