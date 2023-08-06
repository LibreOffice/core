'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    On Error GoTo ErrorHandler ' Set up error handler

    Dim Xml As String
    Dim XmlLen As Long
    ' Not namespace-well-formed XML, parse is expected to fail
    Xml = "<a:xml/>"
    XmlLen = Len(Xml)
    Dim XmlByte(1 To XmlLen) As Byte
    Dim Index As Integer
    For Index = 1 To XmlLen
        XmlByte(Index) = Asc(Mid(Xml, Index, 1))
    Next
    Dim source As Object
    source = CreateUnoStruct("com.sun.star.xml.sax.InputSource")
    source.aInputStream = com.sun.star.io.SequenceInputStream.createStreamFromSequence(XmlByte)
    Dim parser As Object
    parser = CreateUnoService("com.sun.star.xml.sax.FastParser")
    ' Parse crashed before the fix
    parser.ParseStream(source)

    ' Shouldn't end up here
    doUnitTest = "FAIL"
    Exit Function
ErrorHandler:
    doUnitTest = "OK"
End Function
