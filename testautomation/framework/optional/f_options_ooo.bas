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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : General Tools/Options test
'*
'\***********************************************************************

sub main

    GLOBAL_USE_NEW_SLEEP = TRUE
    
    use "framework\optional\includes\options_ooo_general.inc"
    use "framework\optional\includes\options_ooo_memory.inc"
    use "framework\optional\includes\options_ooo_userdata.inc"
    use "framework\optional\includes\options_ooo_view.inc"
    use "framework\optional\includes\options_ooo_print.inc"
    use "framework\optional\includes\options_ooo_paths.inc"
    use "framework\optional\includes\options_ooo_colors.inc"
    use "framework\optional\includes\options_ooo_fontreplacement.inc"
    use "framework\optional\includes\options_ooo_security.inc"
    use "framework\optional\includes\options_ooo_appearance.inc"
    use "framework\optional\includes\options_ooo_accessibility.inc"
    use "framework\optional\includes\options_ooo_java.inc"   
    
    Call hStatusIn ( "framework", "f_options_ooo.bas" )
    
    printlog " 1. part : test if all global options are saved"
    
    Call tOOoUserData
    Call tOOoGeneral
    Call tOOoMemory
    Call tOOoView
    Call tOOoPrint
    Call tOOoPaths
    Call tOOoColors
    Call tOOoFontReplacement
    Call tOOoSecurity
    Call tOOoAppearance
    Call tOOoAccessibility
    Call tOOoJava
    
    Call hStatusOut
    
end sub

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_control_objects.inc"
    use "global\tools\includes\optional\t_key_tools.inc"
    
    use "framework\tools\includes\options_tools.inc"
    
    Call GetUseFiles
    gApplication = "WRITER"
   
end sub

