'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' 
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* Owner : wolfram.garten@oracle.com
'*
'* short description : Graphics Function: Arrange and Align
'*
'\******************************************************************

global datei as string
global Ueber_Text_1 as string
global Ueber_Text_2 as string
global Ueber_Text_3 as string

sub main
    PrintLog "------------------------- g_arrangealign test -------------------------"
    Call hStatusIn ( "Graphics","g_arrangealign.bas")

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\g_arrangealign.inc"

    gApplication = "IMPRESS"
    PrintLog "-------------------------" + gApplication + "-------------------"
    Datei = ConvertPath (gOfficePath + "user\work\" + "anord.odp")
    Call tiDatei_Fuer_Anordnen_Erstellen ' is needed for the following test :-( ueber vars are set here
    Call tdContextSendBackward
    Call tdContextBringForward
    Call tdContextSendToBack
    Call tdContextBringToFront
    Call tdContextInFrontOfObject
    Call tdContextBehindObject
    Call tdContextReverse

    gApplication = "DRAW"
    PrintLog "-------------------------" + gApplication + "-------------------"
    Datei = ConvertPath (gOfficePath + "user\work\" + "anord.odg")
    Call tiDatei_Fuer_Anordnen_Erstellen ' is needed for the following test :-( ueber vars are set here
    Call tdContextSendBackward
    Call tdContextBringForward
    Call tdContextSendToBack
    Call tdContextBringToFront
    Call tdContextInFrontOfObject
    Call tdContextBehindObject
    Call tdContextReverse

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication = "IMPRESS"
    Call GetUseFiles
end sub
