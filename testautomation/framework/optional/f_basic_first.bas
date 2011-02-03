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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : BASIC IDE / Macro dialogs
'*
'\***********************************************************************

sub main

    use "framework\optional\includes\basic_macros.inc"
    use "framework\optional\includes\basic_ide.inc"
    
    
    call hStatusIn ( "framework", "f_basic_first.bas" )
    
    hSetMacroSecurity( GC_MACRO_SECURITY_LEVEL_LOW )

    call tMakro_In_All_Apps       ' open BASIC-IDE in all Applications
    call tMakro_Dialog            ' update-test for macro-dialog
    call tBasic_IDE_Toolbar_Module    ' update-test for Toolbar in Basic-IDE
    call tBasic_IDE_Toolbar_Dialogs   ' check macro bar for a new dialog

    hSetMacroSecurity( GC_MACRO_SECURITY_LEVEL_DEFAULT )

    call hStatusOut

end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"

    use "global\tools\includes\optional\t_treelist_tools.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_basic_ide_tools.inc"
    use "global\tools\includes\optional\t_security_tools.inc"
    use "global\tools\includes\optional\t_basic_organizer_tools.inc"    
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_accels.inc"
    use "global\tools\includes\optional\t_stringtools.inc"

    use "framework\tools\includes\customize_tools.inc"

    gApplication = "WRITER"
    Call GetUseFiles
    
end sub

