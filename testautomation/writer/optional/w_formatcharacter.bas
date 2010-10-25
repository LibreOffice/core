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
'* owner : helge.delfs@oracle.com
'*
'* short description : Formatting of characters, setting styles etc.
'*
'\***********************************************************************

sub main
    Dim StartTime
    StartTime = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\optional\includes\formatcharacter\w_format_character1.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartTime )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"

    Call hStatusIn ( "writer", "w_formatcharacter.bas","Formatting of characters" )

        Call tFormatCharacter
        Call tFormatStyles1                        '-> Format/Styles with contect menu
        Call tFormatCharacter1                '-> Format/Pair Kerning
        Call tFormatCharacter2                '-> Format/Default
        Call tFormatCharacter3                '-> Format/Character
        Call tFormatCharacterOverline1    '-> Format/Overline
        Call tFormatCharacterOverline2    '-> Format/Overline (with save)
        Call tFormatHyperlink                    '-> Autocorrection of hyperlinks

    Call hStatusOut

    Printlog Chr(13) + "End of Test :"
    Printlog "Duration: "+ WieLange ( StartTime )
    Printlog "Date: " +  Date + "    Time: " + Time
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_extension_manager_tools.inc"
    use "global\tools\includes\optional\t_ui_filters.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub
