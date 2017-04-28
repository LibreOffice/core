'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    ' CDateFromIso CDateToIso
    If ( CDateToIso( CDateFromIso("20161016") ) <> "20161016" ) Then
        doUnitTest = 0
    ElseIf ( CDateToIso( CDateFromIso("2016-10-16") ) <> "20161016" ) Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
    ' TODO: add some failure tests for misformed input, On Error whatever?
End Function
