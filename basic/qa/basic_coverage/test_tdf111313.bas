'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as String

    ' Without the fix in place, this test would have crashed
    s = "abc"
    Mid(s,1) = "d"
    If (s = "d") Then
        doUnitTest = "OK"
    Else
        doUnitTest = "FAIL"
    End If
End Function
