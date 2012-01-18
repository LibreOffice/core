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

