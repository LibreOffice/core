'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Sub Main
    Set oContext = GetDefaultContext()
    MsgBox "Connected to a running office..."
    Set oServiceManager=oContext.GetServiceManager()
    If IsNull(oServiceManager) Then
        bAvailable = "not "
    End If
    MsgBox "ServiceManager is " + bAvailable + "available"
End Sub