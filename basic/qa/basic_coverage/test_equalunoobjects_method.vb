' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as String
    ' EqualUnoObjects
    ' Copy of objects -> same instance
    oIntrospection = CreateUnoService( "com.sun.star.beans.Introspection" )
    oIntro2 = oIntrospection
    If ( EqualUnoObjects( oIntrospection, oIntro2 ) = False ) Then
        doUnitTest = "FAIL"
    Else
        ' Copy of structs as value -> new instance
        Dim Struct1 as new com.sun.star.beans.Property
        Struct2 = Struct1
        If ( EqualUnoObjects( Struct1, Struct2 ) ) Then
            doUnitTest = "FAIL"
        Else
            doUnitTest = "OK"
        End If
    End If
End Function
