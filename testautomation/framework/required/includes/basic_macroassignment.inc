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

    printlog( "Update-Test for some dialogs in Basic-IDE" )
    
    dim brc as boolean
    
    hCreateDocument()
    
    printlog( "Create a new BASIC-module for this document" )
    
    brc = hOpenBasicOrganizerFromDoc()
    
    brc = hCreateModuleForDoc( "tUpdtBasicIde" )
    if ( not brc ) then
        warnlog( "Failed to create a new BASIC module for this document" )
        brc = hDestroyDocument()
        goto endsub
    endif
    
    printlog( "Create a new BASIC-dialog" )
    brc = hNewDialog()
    if ( not brc ) then
        warnlog( "Failed to create a new BASIC-dialog" )
        hCloseBasicIde()
        brc = hDestroyDocument()
        goto endsub
    endif
    
    printlog( "Select the Dialog-Pane from the DialogWindow" )
    brc = hSelectDialogPane()
    if ( not brc ) then
        warnlog( "Selecting failed: ToolsCollectionBar is in the way." )
        hCloseBasicIde()
    else
    
        printlog( "Show ToolsCollectionBar" )
        brc = hShowMacroControls()
        if ( not brc ) then
            warnlog( "ToolsCollectionBar is not open" )
        endif
        
        printlog( "Open the Property-Browser: TabGeneralControl" )
        brc = hOpenPropertyBrowser()
        if ( brc ) then
            call dialogtest( TabGeneralControl )
        else
            warnlog( "Property-Browser is not open, skipping further testing" )
        endif
        
        if ( brc ) then
            brc = hSetPBTabPage( 2 )
        endif
        
        if ( brc ) then
        
            printlog( "Current Dialog: Property-Browser / Events-Page" )
            call dialogtest( TabEventsControl )
        
            Kontext "TabEventsControl"
            printlog( "Click the button for the FocusGained-Event" )
            try
                PBFocusGained.click()
            catch
                warnlog( "#i64196# - Events missing on properties tab-page" )
                goto skipevents
            endcatch
        
            Kontext "AssignMacro"
            printlog( "Current Dialog: Assign Macro" )
            call dialogtest( AssignMacro )
            printlog( "Click the Assign-Button on the Macro-Assignment-Dialog" )
            AssignButton.click()
        
            Kontext "ScriptSelector"
            printlog( "Current Dialog: ScriptSelector" )
            call dialogtest( ScriptSelector )
            printlog( "Cancel the scriptselector" )
            ScriptSelector.cancel()
        
            Kontext "AssignMacro" 
            printlog( "Current Dialog: Assign Macro" )
            printlog( "Cancel the Script-Assignement-Dialog" )
            AssignMacro.cancel()
            
        endif
        
        skipevents:
       
        printlog( "Current Dialog: Property-Browser" )
        printlog( "Close the Property-Browser, return to the document" )
        brc = hClosePropertyBrowser()
        
        brc = hCloseBasicIde()
        
    endif
    
    brc = hDestroyDocument()

endcase

