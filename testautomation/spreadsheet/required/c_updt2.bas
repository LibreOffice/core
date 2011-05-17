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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Resource-Test Spreadsheet Application Part II
'*
'\***********************************************************************

sub main

    use "spreadsheet\required\includes\c_upd_formatmenu.inc"
    use "spreadsheet\required\includes\c_upd_formatmenu2.inc"
    use "spreadsheet\required\includes\c_upd_formatmenu3.inc"
    use "spreadsheet\required\includes\c_upd_toolsmenu.inc"
    use "spreadsheet\required\includes\c_upd_toolsmenu2.inc"
    use "spreadsheet\required\includes\c_upd_datamenu.inc"
    use "spreadsheet\required\includes\c_upd_windowmenu.inc"
    use "spreadsheet\required\includes\c_upd_helpmenu.inc"
    use "spreadsheet\tools\includes\c_cell_tools.inc"
    use "spreadsheet\tools\includes\c_select_tools.inc"

    Printlog "-----------------------------------------------------"
    Printlog "--- Resource-Test Spreadsheet Application Part II ---"
    Printlog "-----------------------------------------------------"   

    call hStatusIn("Spreadsheet", "c_updt2.bas", "Resource Test for Spreadsheet Part II")

    call c_upd_formatmenu    'FORMAT Menu Part I
    call c_upd_formatmenu2   'FORMAT Menu Part II
    call c_upd_formatmenu3   'FORMAT Menu Part III    
    call c_upd_toolsmenu     'TOOLS  Menu Part I
    call c_upd_toolsmenu2    'TOOLS  Menu Part II
    call c_upd_datamenu      'DATA   Menu
    call c_upd_windowmenu    'WINDOW Menu
    call c_upd_helpmenu      'HELP   Menu
    
    call hStatusOut

end sub

' ********************************************
' ** - global start routines
' ********************************************
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_locale_tools.inc"
    gApplication   = "CALC"
    GetUseFiles ()
end sub
