'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() as Integer
    Dim A As Integer
    ' In the da-DK locale the comma is the decimal separator and the
    ' period is the grouping separator. The CInt function is
    ' expected to use the locale. Verify that.
    A = CInt("23,456")
    If A <> 23 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
