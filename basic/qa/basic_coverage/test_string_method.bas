' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Dim failedAssertion As Boolean, messages As String

Function doUnitTest ' String(Number As Long, Character As String)
    
    assert(String(3, "H")= "HHH",                           "String(3, ""H"") is not ""HHH""")
    assert(String(5.8, "à")= "àààààà",                      "String(5.8, ""à"") is not ""àààààà""")
    assert(String(Number:=3.45, Character:="test")="ttt",   "String(Number:=3.45, Character:=""test"") is not ""ttt""")
    assert(String(Character:="☺😎", Number:=7)= "☺☺☺☺☺☺☺",  "String(Character:=""☺😎"", Number:=7) is not ""☺☺☺☺☺☺☺""")

    If FailedAssertion Then
        doUnitTest = "test_string_method.vb failed" + messages
        Exit Function
    EndIf
    doUnitTest = "OK" ' All checks passed
End Function

Sub assert(expression As Boolean, errMessage As String)
    if ( Not expression ) Then
       messages = messages + Chr(10) + ErrMessage
       failedAssertion = True
    EndIf
End Sub