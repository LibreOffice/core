'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: f_options_general.bas,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-20 07:07:27 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* owner : thorsten.bosbach@sun.com
'*
'* short description : General Tools/Options test
'*
'\***********************************************************************

sub main

    use "framework\optional\includes\options_so_4.inc"
    use "framework\optional\includes\options_so_5.inc"
    use "framework\optional\includes\options_lan1.inc"
    use "framework\optional\includes\options_int1.inc"
    use "framework\optional\includes\options_ls_2.inc"
    use "framework\optional\includes\options_asia.inc"
    use "framework\optional\includes\options_data.inc"
    
    Call hStatusIn ( "framework", "f_options_general.bas")
    
    ' Reset the autosave feature to 15 min, confirm and disable autosave
    call hResetAutosave()
    
    printlog " 1. part : test if all global options are saved"
    
    call opt_lan1
    Call tInternetProxy
    Call tInternetSearch
    Call tDatasourceConnections
    
    printlog " 2. part : test the functionality of some options"
    Call func_StarOfficeGeneral_1
    Call func_StarOfficeGeneral_2
    Call func_StarOfficeGeneral_3
    Call func_StarOfficeGeneral_4
    Call func_StarOfficeMemory_1
    Call func_StarOfficeView_1
    Call func_StarOfficeView_2
    Call func_StarOfficeView_3
    Call func_StarOfficeView_4
    Call func_StarOfficeView_5
    Call func_StarOfficeView_6
    Call func_StarOfficePrint_1
    Call func_LoadSaveGeneral_1
    Call func_LoadSaveGeneral_2
    Call func_LanguageSettings_Language
    
    ' Reset the autosave feature to 15 min, confirm and disable autosave
    call hResetAutosave()   
    
    Call hStatusOut
   
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    use "framework\tools\includes\options_tools.inc"
    
    Call GetUseFiles
    gApplication = "Writer"
end sub


