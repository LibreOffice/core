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
'* short description : Draw Required Test
'*
'\*****************************************************************

public glLocale (15*20) as string
global ExtensionString as String

sub main
    Printlog "--------------------- Draw Required Test ------------------- "
    Call hStatusIn ( "Graphics","d_updt.bas")

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\required\includes\global\id_001.inc"                        'File
    use "graphics\required\includes\global\id_002.inc"                        'Edit
    use "graphics\required\includes\global\id_003.inc"                        'View
    use "graphics\required\includes\global\id_004.inc"                        'Insert
    use "graphics\required\includes\global\id_005.inc"                        'Format
    use "graphics\required\includes\global\id_006.inc"                        'Tools
    use "graphics\required\includes\global\id_007.inc"                        'Modify
    use "graphics\required\includes\global\id_008.inc"                        'Window
    use "graphics\required\includes\global\id_009.inc"                        'Help
    use "graphics\required\includes\global\id_011.inc"                        'Toolbars
    use "graphics\required\includes\draw\d_002_.inc"
    use "graphics\required\includes\draw\d_003_.inc"
    use "graphics\required\includes\draw\d_005_.inc"
    use "graphics\required\includes\draw\d_007.inc"

    Call d_003
    call id_002
    call id_011
    Call d_002
    call id_003
    call id_004
    Call d_005
    call id_005
    call id_Tools
    call d_007
    call id_007
    call id_008
    call id_009
    call id_001
    Call g_printing
    Call tFileExportAsPDF
    Call tExportAsPDFButton

   Call hStatusOut
end sub
'----------------------------------------------

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\required\includes\g_option.inc"
    use "global\required\includes\g_customize.inc"
    use "global\required\includes\g_001.inc"
    use "global\required\includes\g_printing.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_control_objects.inc"
   Call GetUseFiles
   gApplication   = "DRAW"
end sub

