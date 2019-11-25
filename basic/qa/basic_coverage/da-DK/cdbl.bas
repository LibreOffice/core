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
    ' In the da-DK locale the comma is the decimal separator and the
    ' period is the grouping separator. The CDbl function is
    ' expected to use the locale. Verify that.
    A = CDbl("222.222")
    If A <> 222222 Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    End If
End Function
