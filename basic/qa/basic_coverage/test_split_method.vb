' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as Integer

    doUnitTest = 0

    ' SPLIT
    If ( Split( "Hello world" )(1) <> "world" ) Then Exit Function

    ' tdf#123025 - split function sets the datatype of the array to empty,
    ' preventing any subsequent assignments of values to the array and to the elements itself.
    Dim arr(1) As String
    arr = Split("a/b", "/")
    If ( arr(0) <> "a" Or arr(1) <> "b" ) Then Exit Function
    ReDim Preserve arr(1)
    If ( arr(0) <> "a" Or arr(1) <> "b" ) Then Exit Function
    ReDim arr(1)
    If ( arr(0) <> "" Or arr(1) <> "" ) Then Exit Function
    arr(0) = "a"
    arr(1) = "b"
    If ( arr(0) <> "a" Or arr(1) <> "b" ) Then Exit Function
    ReDim Preserve arr(1)
    If ( arr(0) <> "a" Or arr(1) <> "b" ) Then Exit Function

    doUnitTest = 1

End Function
