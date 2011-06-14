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
' **
' ** owner : gregor.hartmann@oracle.com
' **
' ** short description : Helper functions for Macro tests
' **
'\******************************************************************************

function hInsertMacroFromFile( cMacroId as string, optional cSource as string ) as integer

    '///<h3>Paste a macro (taken from a file) to the basic IDE</h3>
    '///<i>uses: framework/tools/input/macros.txt</i><br>
    '///<i>Starting point: Basic IDE</i><br>
    '///<u>Note</u>: Overwrites any existing macros in the current module<br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name (ID) of the macro to be inserted (string)</li>
    '///<ul>
    '///+<li>Allowed is any string that corresponds to a section in the source file</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Number of lines inserted (integer)</li>
    '///<ul>
    '///+<li>0: Error, no lines inserted</li>
    '///+<li>&gt; 0: Number of lines</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hInsertMacroFromFile::"
    const RETVAL_FAILURE = 0
    const MAX_LINES_IN_MACRO_FILE = 10000
    
    '///+<li>Find the path to the source file</li>
    dim cFile as string
    dim aInstructionList( MAX_LINES_IN_MACRO_FILE ) as string
    dim iInstructionCount as integer
    dim iCurrentInstruction as integer

    if ( IsMissing( cSource ) ) then cFile = gTesttoolPath & "global/input/macros.txt"

    '///+<li>retrieve the macro from the file with ID as section</li>
    iInstructionCount = hGetDataFileSection( cFile, aInstructionList(), cMacroId, "", "" )
                        
    '///+<li>Delete all content from the BASIC IDE edit window</li>
    '///+<li>Insert the code into the IDE line by line</li>
    kontext "BasicIDE"
    if ( EditWindow.exists() ) then
        if ( hDeleteMacro() ) then
            for iCurrentInstruction = 1 to iInstructionCount
                EditWindow.TypeKeys( "<HOME>" )
                EditWindow.TypeKeys( aInstructionList( iCurrentInstruction ) )
                EditWindow.TypeKeys( "<RETURN>" )
            next iCurrentInstruction
            printlog( CFN & "Inserted macro: " & cMacroId )
            hInsertMacroFromFile() = iInstructionCount
        else
            printlog( CFN & "IDE is not empty, will not insert macro" )
            hInsertMacroFromFile() = RETVAL_FAILURE
        endif
    else
        printlog( CFN & "Editwindow is not visible" )
        hInsertMacroFromFile() = RETVAL_FAILURE
    endif
    '///</ul>
        
end function

'*******************************************************************************

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