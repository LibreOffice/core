'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: f_topten.bas,v $
'*
'* $Revision: 1.3 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-20 08:07:03 $
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
'/******************************************************************************
'*
'* owner : joerg.skottke@sun.com
'*
'* short description : smoke test for each application
'*
'\******************************************************************************

sub main

    use "framework\required\includes\topten.inc"
    use "framework\required\includes\smoketest.inc"
        
    call hStatusIn ("framework", "f_topten.bas")
    
    printlog Chr(13) + "* ------       Top-Ten-Test       ------ *"
    call topten
    
    
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_LOW )
    call tSmokeTest
    
    ' Do not use hSetMacroSecurityAPI here as this is the ONLY place within
    ' the framework module where the setting is done via UI.    
    hSetMacroSecurity( GC_MACRO_SECURITY_LEVEL_DEFAULT )
    
    call hStatusOut
end sub

'*******************************************************************************

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    use "global\tools\includes\optional\t_docfuncs.inc"
   
    gApplication = "BACKGROUND"
    Call GetUseFiles
    
end sub
