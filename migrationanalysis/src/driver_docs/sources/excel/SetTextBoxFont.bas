Attribute VB_Name = "SetTextBoxFont"
'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'
' This file incorporates work covered by the following license notice:
'
'   Licensed to the Apache Software Foundation (ASF) under one or more
'   contributor license agreements. See the NOTICE file distributed
'   with this work for additional information regarding copyright
'   ownership. The ASF licenses this file to you under the Apache
'   License, Version 2.0 (the "License"); you may not use this file
'   except in compliance with the License. You may obtain a copy of
'   the License at http://www.apache.org/licenses/LICENSE-2.0 .
'

Option Explicit

' We change the font used for text box shapes here for the japanese
' version, because office 2000 sometimes displays squares instead of
' chars
Public Sub SetTextBoxFont()
    Dim aSheet As Worksheet
    Dim myShape As Shape
    Set aSheet = Sheets(1)

    For Each myShape In aSheet.Shapes
        If myShape.Type = msoTextBox Then
            myShape.Select
            With Selection.Characters.Font
                .Name = "MS PGothic"
                .Size = 10
            End With
        End If
    Next myShape
    Range("A1").Select
End Sub

