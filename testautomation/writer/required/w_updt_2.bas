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
'* short description : Ressource-Test Writer Document Part 2 / 2
'*
'\******************************************************************

sub main
    use "writer\tools\includes\w_tools.inc"                  		' global subroutines for Writer-application
    use "writer\tools\includes\w_tool1.inc"                  		' global subroutines for Writer-application
    use "writer\tools\includes\w_tool2.inc"                  		' global subroutines for Writer-application

    use "writer\required\includes\w_006_.inc"
    use "writer\required\includes\w_007_.inc"
    use "writer\required\includes\w_008_.inc"
    use "writer\required\includes\w_009_.inc"
    use "writer\required\includes\w_010_.inc"
    use "writer\required\includes\w_010_1.inc"
    use "writer\required\includes\w_011_.inc"
    use "writer\required\includes\w_020_.inc"
    
    Printlog "******* Ressource-Test Writer Document Part 2 / 2 *******"

    Call wChangeDefaultView()

    Call hStatusIn ( "writer", "w_updt_2.bas" )
    Call w_006_              ' Menu Table
    Call w_007_              ' Menu Tools
    Call w_008_              ' Menu Window
    Call w_009_              ' Menu Help
    Call w_010_              ' Objectbar
    Call w_010_1             ' Objectbar. Part 2
    Call w_011_              ' Rest of Objectbar
    Call w_020_              ' Toolbar
    Call hStatusOut

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\required\includes\g_option.inc"                	' global subroutines for Tools / Options
    use "global\required\includes\g_findbar.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_control_objects.inc"

    gApplication   = "WRITER"
    GetUseFiles ()
end sub
