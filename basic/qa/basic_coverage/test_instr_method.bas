' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String

    doUnitTest = "FAIL"

    Dim aString As Variant
    aString = "Hello"
    ' InStr
    If (InStr(1, aString, "l", 1) <> 3) Then Exit Function

    ' tdf#139840 - case-insensitive operation for non-ASCII characters
    If (InStr(1, "α", "Α", 1) <> 1) Then Exit Function
    ' tdf#139840 - German Eszett is uppercased to a two-character 'SS'.
    ' This test should fail after tdf#110003 has been fixed.
    If (InStr(2, "Straße", "s", 1) <> 5) Then Exit Function

    ' Start position is greater than the length of the string being searched.
    If (InStr(2, "α", "Α", 1) <> 0) Then Exit Function

    doUnitTest = "OK"

End Function
