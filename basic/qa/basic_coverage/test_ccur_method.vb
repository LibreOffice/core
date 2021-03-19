'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer

    doUnitTest = 0

    ' CCUR
    if (CCur("100") <> 100) Then Exit Function
    ' tdf#141050 - passing a number with + sign
    if (CCur("+100") <> 100) Then Exit Function
    ' tdf#141050 - passing a number with - sign
    if (CCur("-100") <> -100) Then Exit Function

    doUnitTest = 1

End Function
