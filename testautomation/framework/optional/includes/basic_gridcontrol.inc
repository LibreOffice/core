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
'* short description : gridcontrol
'*
'\******************************************************************************

testcase tGridcontrolLoad

    const MACRO_NAME = "VclTestTool"
    const MACRO_NOT_FOUND = 0
    const FILE_NAME  = "framework/optional/input/gridcontrol.odt"

    dim iPos as integer
    
    printlog( "Open the test document: " & FILE_NAME )
    call hFileOpen( gTestToolPath & FILE_NAME )

    printlog( "Open the <Run Macro> dialog" )
    ToolsMacrosRunMacro

    printlog( "Find the document, its library and the test macro, run the macro" )
    Kontext "ScriptSelector"
    if ( ScriptSelector.exists( 10 ) ) then

    printlog "Start the macro, that performs the test"
    Kontext "GridControlDialogStarter"
 ' FAILS IF DOCUMENT IS READONLY....
    ShowGridcontrol.typeKeys "<space>"
    Kontext "GridControlDialog"
    if GridControlDialog.exists(5) then
        for i=1 to gridcontrolcontrol.getcolumncount
        for a=1 to gridcontrolcontrol.getrowcount
            printlog " " + i + ":"+a+": '" + gridcontrolcontrol.getitemtype (i,a) + "' '" + gridcontrolcontrol.getitemtext (i,a) + "'"
        next a
        next i
        gridcontrolcontrol.select 5

        kontext
        if active.exists(5) then
            printlog active.gettext
            active.ok
        else
            warnlog "No selection Event by .select"
        endif

    Kontext "GridControlDialog"
        gridcontrolcontrol.typeKeys("<down>")

        kontext
        if active.exists(5) then
        printlog active.gettext
        active.ok
        else
        warnlog "No selection event by .typeKeys <down>"
        endif
    Kontext "GridControlDialog"
        gridcontrolcontrol.typeKeys("<up>")
        kontext
        if active.exists(5) then
        printlog active.gettext
        active.ok
        else
        warnlog "No selection event by .typeKeys <up>"
        endif

    Kontext "GridControlDialog"
        GridControlDialog.close
    else
        warnlog "Gridcontrol Dialog did not come up after pressing button"
    endif
    
    printlog( "Test exit, cleanup" )
    hFileCloseAll()
    
endcase


