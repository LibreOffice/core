'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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

