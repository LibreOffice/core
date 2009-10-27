'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: scripting_basics.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
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
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Quick short test for the scripting framework
'*
'\******************************************************************************

testcase tScripting

    
    const ORGANIZER_ITEMS = 2
    
    hInitBackingMode()

    ToolsMacrosOrganizeMacrosBeanShell
    
    Kontext "ScriptOrganizer"
    if ( ScriptOrganizer.exists( 5 ) ) then
        printlog( "Beanshell Script Organizer is present. Good" )
        if ( ScriptTreeList.getItemCount() <> ORGANIZER_ITEMS ) then
            warnlog( "The Treelist does not contain the expected number " & _
            "of items. Please verify that exactly three top-nodes exist" )
        endif
        ScriptOrganizer.cancel()
    else
        warnlog( "Script Organizer for Beanshell is missing" )
    endif
    
    ToolsMacrosOrganizeMacrosJavaScript
    
    Kontext "ScriptOrganizer"
    if ( ScriptOrganizer.exists( 5 ) ) then
        printlog( "JavaScript Script Organizer is present. Good" )
        if ( ScriptTreeList.getItemCount() <> ORGANIZER_ITEMS ) then
            warnlog( "The Treelist does not contain the expected number " & _
            "of items. Please verify that exactly three top-nodes exist" )
        endif
        ScriptOrganizer.cancel()
    else
        warnlog( "Script Organizer for JavaScript is missing" )
    endif
    
    
endcase
    


