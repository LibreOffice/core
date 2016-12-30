'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    dim aDate as Date
    dim aTime as Date
    aDate = Date()
    aTime = Time()
    ' NOW TIMEVALUE
    If ( Now() < aDate + TimeValue(aTime) ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function
