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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Delete modules while BASIC is running
'*
'\******************************************************************************

testcase tDeleteModulesAtRunningBasic

    printlog( "Delete BASIC module while BASIC is running" )

    const RAISE_MESSAGEBOX = 1

    dim brc as boolean
    const MAX_WAIT = 3000
    dim iMenuItemCount as integer

    printlog( "Open a new writer-doc" )
    gApplication = "WRITER"
    Call hNewDocument()

    printlog( "Open the BASIC Organizer" )
    ToolsMacro_uno

    printlog( "Create a new module for the current document" )
    brc = hCreateModuleForDoc()
    if ( not brc ) then
        warnlog( "Unable to create a basic module for the document" )
        goto endsub
    endif

    printlog( "Insert a short script (1. page)" )
    brc = hInsertMacro( RAISE_MESSAGEBOX )
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
        iMenuItemCount = hMenuItemGetCount()

        select case ( iMenuItemCount )
        case 2    : printlog( "Correct number of items in menu: " & iMenuItemCount )
        case 3    : warnlog ( "Delete is active in context menu for a running macro => BUG!" )
        case 5    : warnlog ( "#i101972# - Disabled entries are shown in context menu on tab bar" )
        case else : warnlog ( "Incorrect itemcount in context menu: " & iMenuItemCount )
        end select
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

    Call hFileCloseAll()

endcase

