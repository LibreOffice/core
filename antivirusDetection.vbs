machineName = "."
Set objWMIService = GetObject("winmgmts:{impersonationLevel=impersonate}!\\" & machineName & "\root\SecurityCenter2")
Set installedAntiviruses = objWMIService.ExecQuery("Select * from AntivirusProduct")
For Each antivirus in installedAntiviruses
    If  antivirus.productState And &h01000 Then
    MsgBox antivirus.displayName & " is active on your system and it is highly recommended that you exclude the build directories associated with LibreOffice.",48,"Antivirus exclusions"
     End if
Next

