'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

' cf. examples at <https://docs.microsoft.com/en-us/dotnet/visual-basic/language-reference/
' statements/mid-statement>:
Function doUnitTest as Integer
    s = "The fox jumps"
    Mid(s, 5, 3, "duck")
    If (s = "The duc jumps") Then
        doUnitTest = 1
    Else
        doUnitTest = 0
    End If
End Function
