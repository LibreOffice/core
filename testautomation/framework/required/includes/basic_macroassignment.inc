'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_macroassignment.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:03 $
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
'*  short description : Update-Test for some dialogs in Basic-IDE
'*
'\******************************************************************************

testcase tUpdtMacroAssignment

    '///<H1>Update test for some dialogs in Basic-IDE</H1>
    '///<ul>
    
    dim brc as boolean
    
    '///+<li>Create a new document</li>
    hCreateDocument()
    
    printlog( "Create a new BASIC-module for this document" )
    
    '///+<li>Open the Basic organizer</li>
    brc = hOpenBasicOrganizerFromDoc()
    
    '///+<li>Create a new module for the current document -> Basic IDE</li>
    brc = hCreateModuleForDoc( "tUpdtBasicIde" )
    if ( not brc ) then
        warnlog( "Failed to create a new BASIC module for this document" )
        brc = hDestroyDocument()
        goto endsub
    endif
    
    '///+<li>Create a new Basic dialog</li>
    printlog( "Create a new BASIC-dialog" )
    brc = hNewDialog()
    if ( not brc ) then
        warnlog( "Failed to create a new BASIC-dialog" )
        hCloseBasicIde()
        brc = hDestroyDocument()
        goto endsub
    endif
    
    '///+<li>Select the empty dialogpane in the dialog editor</li>
    printlog( "Select the Dialog-Pane from the DialogWindow" )
    brc = hSelectDialogPane()
    if ( not brc ) then
        warnlog( "Selecting failed: ToolsCollectionBar is in the way." )
        hCloseBasicIde()
    else
    
        '///+<li>Open the ToolsCollectionBar (Basic controls)</li>
        printlog( "Show ToolsCollectionBar" )
        brc = hShowMacroControls()
        if ( not brc ) then
            warnlog( "ToolsCollectionBar is not open" )
        endif
        
        '///+<li>Open the Property Browser (Properties page)</li>
        printlog( "Open the Property-Browser: TabGeneralControl" )
        brc = hOpenPropertyBrowser()
        if ( brc ) then
            call dialogtest( TabGeneralControl )
        else
            warnlog( "Property-Browser is not open, skipping further testing" )
        endif
        
        '///+<li>Switch to the Events Page</li>
        if ( brc ) then
            brc = hSetPBTabPage( 2 )
        endif
        
        if ( brc ) then
        
            printlog( "Current Dialog: Property-Browser / Events-Page" )
            call dialogtest( TabEventsControl )
        
            '///+<li>Click on the button for the Focus-Gained event</li>
            Kontext "TabEventsControl"
            printlog( "Click the button for the FocusGained-Event" )
            try
                PBFocusGained.click()
            catch
                warnlog( "#i64196# - Events missing on properties tab-page" )
                goto skipevents
            endcatch
        
            '///+<li>Click the Assign button on the Assign Macro dialog</li>
            Kontext "AssignMacro"
            printlog( "Current Dialog: Assign Macro" )
            call dialogtest( AssignMacro )
            printlog( "Click the Assign-Button on the Macro-Assignment-Dialog" )
            AssignButton.click()
        
            '///+<li>Close the ScriptSelector with Cancel</li>
            Kontext "ScriptSelector"
            printlog( "Current Dialog: ScriptSelector" )
            call dialogtest( ScriptSelector )
            printlog( "Cancel the scriptselector" )
            ScriptSelector.cancel()
        
            '///+<li>Close the Assign Macro dialog with Cacnel</li>
            Kontext "AssignMacro" 
            printlog( "Current Dialog: Assign Macro" )
            printlog( "Cancel the Script-Assignement-Dialog" )
            AssignMacro.cancel()
            
        endif
        
        skipevents:
       
        '///+<li>Close the Property Browser</li>
        printlog( "Current Dialog: Property-Browser" )
        printlog( "Close the Property-Browser, return to the document" )
        brc = hClosePropertyBrowser()
        
        '///+<li>Close the Basic IDE</li>
        brc = hCloseBasicIde()
        
    endif
    
    '///+<li>Close the document</li>
    brc = hDestroyDocument()
    '///</ul>      

endcase

