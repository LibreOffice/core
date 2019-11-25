'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() as String
    Dim A As Double
    A = "222,222"
    ' In the da-DK locale the comma is the decimal separator but we
    ' don't do locale-dependent conversion for an implicit conversion
    ' as above. Instead we just stop at the comma and don't even treat
    ' it as a grouping separator. This is the traditional OOo way.
    If A <> 222.0 Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    End If
End Function
