'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

' cf. <https://bugs.documentfoundation.org/show_bug.cgi?id=62090> "Mid statement doesn't work as
' expected":
Function doUnitTest as Integer
    s = "The lightbrown fox"
    Mid(s, 5, 10, "lazy")
    If (s = "The lazy fox") Then
        doUnitTest = 1
    Else
        doUnitTest = 0
    End If
End Function
