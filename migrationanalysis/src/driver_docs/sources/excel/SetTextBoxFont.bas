Attribute VB_Name = "SetTextBoxFont"
'/*************************************************************************
' *
' * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' * 
' * Copyright 2008 by Sun Microsystems, Inc.
' *
' * OpenOffice.org - a multi-platform office productivity suite
' *
' * $RCSfile: SetTextBoxFont.bas,v $
' *
' * This file is part of OpenOffice.org.
' *
' * OpenOffice.org is free software: you can redistribute it and/or modify
' * it under the terms of the GNU Lesser General Public License version 3
' * only, as published by the Free Software Foundation.
' *
' * OpenOffice.org is distributed in the hope that it will be useful,
' * but WITHOUT ANY WARRANTY; without even the implied warranty of
' * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' * GNU Lesser General Public License version 3 for more details
' * (a copy is included in the LICENSE file that accompanied this code).
' *
' * You should have received a copy of the GNU Lesser General Public License
' * version 3 along with OpenOffice.org.  If not, see
' * <http://www.openoffice.org/license.html>
' * for a copy of the LGPLv3 License.
' *
' ************************************************************************/

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

