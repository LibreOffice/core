'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_delete_modules.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
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
'*  short description : Delete modules while BASIC is running
'*
'\******************************************************************************

testcase tDeleteModulesAtRunningBasic

    printlog( "Delete BASIC module while BASIC is running" )

    dim brc as boolean
    const MAX_WAIT = 3000
    

    printlog( "Open a new writer-doc" )
    gApplication = "WRITER"

    Call hNewDocument()

    printlog( "Create a new module " )
    brc = hOpenBasicOrganizerFromDoc()
    if ( not brc ) then
        warnlog( "Unable to open BASIC organizer for the document" )
        goto endsub
    endif
    
    brc = hCreateModuleForDoc()
    if ( not brc ) then
        warnlog( "Unable to create a basic module for the document" )
        goto endsub
    endif

    printlog( "Insert a short script (1. page)" )
    brc = hInsertMacro( 1 )
    if ( not brc ) then
        warnlog( "Unable to create a basic module for the document" )
        goto endsub
    endif

    kontext "macrobar"
    printlog( "Click button:  Step Procedure" )
    ProcedureStep.Click()
    if ( WaitSlot( MAX_WAIT ) <> WSFinished ) then
        warnlog( "Slot not finished, timeout reached" )
    endif

    kontext "basicide"
    printlog( "Check if disabled menu items are shown in context menu and 'delete' is not shown" )
    Tabbar.OpenContextMenu()

    try
        if ( hMenuItemGetCount() = 5 ) then
            warnlog( "#i101972# - Disabled entries are shown in context menu on tab bar" )
        end if


        if ( hMenuItemGetCount() = 3 ) then
            warnlog( "Delete is active in context menu for a running macro => BUG!" )
        end if

    catch
        warnlog( "Unable to retrieve the number of menuitems" )
    endcatch

    hMenuClose()
    printlog( "Stop the running script" )

    kontext "macrobar"
    BasicStop.Click()
    if ( WaitSlot( MAX_WAIT ) <> WSFinished ) then
        warnlog( "Slot not finished, timeout reached" )
    endif

    Call hCloseDocument()
    Call hCloseDocument()

endcase

