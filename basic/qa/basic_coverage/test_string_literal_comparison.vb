' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer
    ' tdf#142180 - Invalid text comparison result in Basic

    doUnitTest = 0
    If ( "Z" < "A" ) Then Exit Function
    If ( "A" > "Z" ) Then Exit Function
    If ( "A" < "A" ) Then Exit Function
    If ( "A" > "A" ) Then Exit Function
    If ( "Z" <= "A" ) Then Exit Function
    If ( "A" >= "Z" ) Then Exit Function
    doUnitTest = 1
End Function
