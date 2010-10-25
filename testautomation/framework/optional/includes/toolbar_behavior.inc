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
'* short description : Resource test of toolbar behavior
'*
'\************************************************************************

testcase tToolbarBehavior
    hNewDocument()
    sleep (5)
        'ViewToolbarsReset
        hUseMenu()
        printlog hMenuSelectNr(3)
        printlog hMenuSelectNr(3)
        printlog hMenuSelectNr(-1)
        kontext
        if active.exists(5) then
            active.setPage TabCustomizeMenu
            if TabCustomizeMenu.exists(5) then
                printlog "All Tooolbars are available."
                TabCustomizeMenu.cancel
            else
                printlog "Resetet Toolbars."
            endif
        else
            printlog "Toolbars resetet"
        endif
    kontext "TableObjectBar"
    if TableObjectBar.exists(5) then
        warnlog "Toolbar is already available?"
    else
        printlog "Toolbar is not available"
    endif
    InsertTableWriter
    kontext "TabelleEinfuegenWriter"
    if TabelleEinfuegenWriter.exists (5) then
        TabelleEinfuegenWriter.ok
        sleep (2)
    else
        warnlog "no table"
    endif
    kontext "TableObjectBar"
    if TableObjectBar.exists(5) then
        printlog "Toolbar is available."
    else
        warnlog "Toolbar is not available"
    endif

    if NOT TableObjectBar.isDocked then
    	TableObjectBar.dock
    	printlog "Toolbar got docked, to use thier kontext menu"
    endif
        TableObjectBar.openContextMenu
        printlog hMenuSelectNr(-1)
        kontext "TableObjectBar"
        if TableObjectBar.exists(5) then
            warnlog "Toolbar is still available"
        else
            printlog "Toolbar is not available"
        endif
        hTypeKeys("<mod1 end><mod1 end>")
        kontext "TableObjectBar"
        if TableObjectBar.exists(5) then
            warnlog "Toolbar is still available"
        else
            printlog "Toolbar is not available"
        endif
    hTypeKeys("<mod1 home>")
    kontext "TableObjectBar"
    if TableObjectBar.exists(5) then
        printlog "Toolbar is available."
    else
        warnlog "Toolbar is not available"
    endif

    Call hToolbarSelect ( "Table", false )
    kontext "TableObjectBar"
    if TableObjectBar.exists(5) then
        warnlog "Toolbar is still available"
    else
        printlog "Toolbar is not available"
    endif

    hTypeKeys("<mod1 end><mod1 end>")
    kontext "TableObjectBar"
    if TableObjectBar.exists(5) then
        warnlog "Toolbar is available again"
    else
        printlog "Toolbar is not available"
    endif

    hTypeKeys("<mod1 home>")
    kontext "TableObjectBar"
    if TableObjectBar.exists(5) then
        warnlog "Toolbar is available again"
    else
        printlog "Toolbar is not available"
    endif
    
        'ViewToolbarsReset
        hUseMenu()
        printlog hMenuSelectNr(3)
        printlog hMenuSelectNr(3)
        printlog hMenuSelectNr(-1)
        kontext
        if active.exists(5) then
            active.setPage TabCustomizeMenu
            if TabCustomizeMenu.exists(5) then
                warnlog "Can't reset toolbars"
                TabCustomizeMenu.cancel
            else
                printlog "Resetet Toolbars."
            endif
        else
            printlog "Toolbars resetet"
        endif
    kontext "TableObjectBar"
    if TableObjectBar.exists(5) then
        printlog "Toolbar is available."
    else
        warnlog "Toolbar is not available"
    endif

    hCloseDocument()
endcase

