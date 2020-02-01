machineName = "."
Set objWMIService = GetObject("winmgmts:{impersonationLevel=impersonate}!\\" & machineName & "\root\SecurityCenter2")
Set installedAntiviruses = objWMIService.ExecQuery("Select * from AntivirusProduct")
For Each antivirus in installedAntiviruses
    If  antivirus.productState And &h01000 Then
    MsgBox antivirus.displayName & " is active and it is highly recommended that you disable it.",16,"DISABLE ANTIVIRUS"
     End if
Next

