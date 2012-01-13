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

*****

function hMacroOrganizerRunMacro( cMacroName as string ) as integer

    ' Run a macro by name. Returnvalues as follows:
    ' = 0: any error
    ' > 0: position of macro in the list, success.

    const CFN = "hMacroOrganizerRunMacro::"
    const RETVAL_FAILURE = 0

    dim iMacroPosition as integer

    ToolsMacro_uno

    kontext "Makro"
    if ( Makro.exists( 2 ) ) then
        hExpandAllNodes( MakroAus )
        iMacroPosition = hSelectNodeByName( MakroAus, cMacroName )
        if ( iMacroPosition ) then
            iMacroPosition = hSelectNextNode( MakroAus )
            if ( hClickButton( Ausfuehren ) ) then
                hMacroOrganizerRunMacro() = iMacroPosition
            else
                ' run macro button not enabled
                warnlog( CFN & "Unable to click <Run macro> button" )
                hMacroOrganizerRunMacro() = RETVAL_FAILURE
            endif
        else
            warnlog( CFN & "Macro was not found" )
            hMacroOrganizerRunMacro() = RETVAL_FAILURE
        endif
    else
        warnlog( CFN & "Failed to open Macro Organizer" )
        hMacroOrganizerRunMacro() = RETVAL_FAILURE
    endif

    
end function

'*******************************************************************************

function hBasicIDERunMacro( cIdentifier as string ) as boolean

    const CFN = "hBasicIDERunMacro(): "
    const MAX_WAIT = 1
    dim cMessage as string : cMessage = ""

    kontext "BasicIDE"
    if ( BasicIDE.exists() ) then

        Wait( 2000 )

        kontext "MacroBar"
        if ( MacroBar.exists() ) then
            BasicRun.click()
        else
            warnlog( "MacroBar not accessible" )
        endif

        kontext "Active"
        if ( Active.exists( MAX_WAIT ) ) then
            cMessage = Active.getText()
            hCloseDialog( Active, "ok" )
        endif

        if ( cMessage = cIdentifier ) then
            printlog( "The expected macro was executed" )
            hBasicIDERunMacro() = true
        else
            printlog( CFN & "Incorrect macro executed" )
            hBasicIDERunMacro() = false
        endif
    else
        warnlog( CFN & "BasicIDE is not open" )
        hBasicIDERunMacro() = false
    endif

end function

'*******************************************************************************

function hExecMacro( cMacro as string ) as boolean

    const CFN = "global::tools::includes::optional::t_macro_tools.inc::hExecMacro(): "

    dim iCurrentModule as integer
    dim iCurrentMacro as integer
    hEXECMacro() = FALSE

    ToolsMacro_uno

    kontext "Makro"
    if ( Makro.exists( 2 ) ) then
        ' Iterate through the left treelist
        for iCurrentModule = hExpandAllNodes( MakroAus ) to 1 step -1
            MakroAus.select( iCurrentModule )
            ' Iterate through the Macros list (right pane)
            for iCurrentMacro = 1 to MakroListe.getItemCount()
                MakroListe.select( iCurrentMacro )
                ' Try to find the macro
                if ( cMacro = MakroListe.getSelText() ) then
                    ' Run the macro
                    if ( Ausfuehren.isEnabled() ) then
                        Ausfuehren.click()
                        hExecMacro() = TRUE
                        exit function
                    else
                        warnlog( CFN & "Macro found but <Ausfuehren> is not enabled" )
                        exit function
                    endif
                endif
            next iCurrentMacro
        next iCurrentModule
    else
        warnlog( CFN & "Macro Organizer is not open within 2 seconds" )
    endif

end function