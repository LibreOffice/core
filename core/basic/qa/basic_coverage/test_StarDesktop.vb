'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
Const IMPL = "com.sun.star.comp.framework.Desktop"
    ' STARDESKTOP
	If (StarDesktop.ImplementationName <> IMPL) Then
        doUnitTest = 0
    Else
        doUnitTest = 1 ' Ok
    End If
End Function
