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
'*  short description : Update-Test for some dialogs in Basic-IDE
'*
'\******************************************************************************

testcase tUpdtMacroAssignment

    printlog( "Update test all dialogs from plain document via assign macro dialog to scriptselector" )
    
    const TABPAGE_EVENTS = 2
    const BASIC_MODULE = "tUpdtMacroAssignment"
    const DEFAULT_DIALOG_TIMEOUT = 1
    const RC_TIMEOUT = -1

    printlog( "New document" )
    hCreateDocument()
    
    printlog( "Create a new BASIC-module for this document" )
    hInitBasicIDE( BASIC_MODULE )
    
    printlog( "Create a new BASIC-dialog" )
    if ( hNewDialog() ) then

        printlog( "Select the Dialog-Pane from the DialogWindow" )
        if ( hSelectDialogPane() ) then

            printlog( "Show ToolsCollectionBar" )
            if ( hShowMacroControls() ) then

                printlog( "Open the Property-Browser" )
                if ( hOpenPropertyBrowser() ) then

                    kontext "TabGeneralControl"
                    call dialogtest( TabGeneralControl )

                    printlog( "Switch to events-tab" )
                    if ( hSetPBTabPage( TABPAGE_EVENTS ) ) then

                        printlog( "Current Dialog: Property-Browser / Events-Page" )
                        call dialogtest( TabEventsControl )

                        Kontext "TabEventsControl"
                        printlog( "Click the button for the FocusGained-Event" )
                        if ( hClickButton( PBFocusGained ) <> RC_TIMEOUT ) then

                            Kontext "AssignMacro"
                            printlog( "Current Dialog: Assign Macro" )
                            if ( AssignMacro.exists( DEFAULT_DIALOG_TIMEOUT ) ) then

                                call dialogtest( AssignMacro )

                                printlog( "Click the Assign-Button on the Macro-Assignment-Dialog to open Script Selector" )
                                if ( hClickButton( AssignButton ) <> RC_TIMEOUT ) then

                                    Kontext "ScriptSelector"
                                    if ( ScriptSelector.exists( DEFAULT_DIALOG_TIMEOUT ) ) then

                                        call dialogtest( ScriptSelector )

                                        printlog( "Close Script Selector" )
                                        hCloseDialog( ScriptSelector, "cancel" )

                                        printlog( "Close all remaining dialogs and documents" )
                                        Kontext "AssignMacro"
                                        hCloseDialog( AssignMacro, "cancel" )
                                        hClosePropertyBrowser()
                                        hCloseBasicIDE()
                                        hDestroyDocument()
                                    else
                                        warnlog( "Failed to open/access <ScriptSelector>" )
                                    endif
                                else
                                    warnlog( "Assign-button is not enabled" )
                                endif
                            else
                                warnlog( "Failed to open/access <AssignMacro> dialog" )
                            endif
                        else
                            warnlog( "#i64196# - Events missing on properties tab-page" )
                        endif
                    else
                        warnlog( "Failed to switch tabpage to Tab-Events" )
                    endif
                else
                    warnlog( "Property-Browser is not open, skipping further testing" )
                endif
            else
                warnlog( "ToolsCollectionBar is not open" )
            endif
        else
            warnlog( "Selecting failed: ToolsCollectionBar is in the way." )
        endif
    else
        warnlog( "Failed to create a new BASIC-dialog" )
    endif

endcase

