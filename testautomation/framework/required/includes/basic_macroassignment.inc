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

