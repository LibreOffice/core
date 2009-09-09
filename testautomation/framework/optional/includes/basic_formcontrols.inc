'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_formcontrols.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
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
'* owner : joerg.skottke@sun.com
'*
'* short description : Basic form-controls
'*
'\******************************************************************************

testcase tAllControlsOnDialog( cMajor as string )

    printlog( "Insert all available controls into a BASIC dialog and verify that changed settings persist a save/load cycle" )

    dim iApplication as Integer ' numeric expression for gApplication
    dim cApplication as string  ' Name of the current application
    
    dim lsControlNames( 30 ) as string  ' list containing the names of the controls

    ' number of controls and the name of the current control
    dim iCurrentControl as integer
    dim sCurrentControl as string

    ' the workfile and the filter (Tested for 569, 645, 680)
    dim sFile as string

    ' errorstatus: if false, something went wrong and the test cancels
    ' trying to recover somehow
    dim bOpen as boolean
    dim brc as boolean 
    dim irc as integer
    
    const CMODULE = "tacod" ' abbreviation for tAllControlsOnDialog

    printlog( "Perform the test for one application only: WRITER" )
    for iApplication = 1 to 1
    
        printlog( "Set document type" )
        cApplication = hNumericDoctype( iApplication )

        printlog( "Build the filename" )
        sFile = hGetWorkPath() & "basic" & hGetSuffix( cMajor )
        sFile = ConvertPath( sFile )
        printlog( "Using file: " & sFile )
        
        printlog( "Delete the file, if it exists" )
        hDeleteFile( sFile )
                
        printlog( "Open a second document" )
        printlog( "" )
        printlog( cApplication )
        brc = hCreateDocument()
        if ( not brc ) then
            warnlog( "Failed to create a new document, aborting" )
            goto endsub
        endif

        printlog( "Create a new module for the new document" )
        printlog( "Create a new dialog in BasicIDE" )
        printlog( "Open the macro controls float" )
        bOpen = hInitFormControls( CMODULE )
        if ( not bOpen ) then
            printlog( "Aborting due to previous errors" )
            call hCloseDocument()
            goto endsub
        endif

        printlog( "Insert all controls into the dialog" )
        for iCurrentControl = 1 to ICONTROLCOUNT

            sCurrentControl = hDrawControlOnDialog( iCurrentControl )
            listappend( lsControlNames() , sCurrentControl )
            hOpenPropertyBrowser()
            hPBSetControlName( sCurrentControl )
            hClosePropertyBrowser()

        next iCurrentControl
        

        Kontext "BASICIDE"
        printlog( "close the BASIC-IDE" )
        hCloseBasicIde()
        
        
        printlog( "" )
        printlog( "Test case 1: Verify that settings survive a close and reopen of the BASIC IDE" )
        
        ToolsMacro_uno
        kontext "Makro"
        irc = hSelectNodeByName( MakroAus , CMODULE )
        if ( irc = 0 ) then
            qaerrorlog( "Searching node by name failed. Using fallback" )
            hSelectTheLastnode( MakroAus )
        else
            printlog( "The node was found. Good." )
        endif
        
        brc = hIsMacroEditButtonEnabled()
        if ( not brc ) then
            warnlog( "Edit-Button is not enabled, the test cannot continue" )
            kontext "Makro"
            Makro.cancel()
            brc = hDestroyDocument()
            goto endsub
        endif
            
        printlog( "Click 'Edit' to edit the module" )
        printlog( "Edit the module" )
        Bearbeiten.Click()
        if ( WaitSlot <> WSFinished ) then
            warnlog( "Slot not finished after 1 second" )
        endif

        kontext "BasicIDE"
        printlog( "Find the dialog we created before" )
        ' try to find our dialog again by stepping through the tabbar
        bOpen = hFindFirstDialog()
        if ( not bOpen ) then
            warnlog( "Could access dialog, aborting test" )
            hCloseBasicIde()
            brc = hDestroyDocument()
            goto endsub
        endif

        printlog( "Open the macro controls float" )
        ' if the dialog is open, open the macro controls toolbar as well,
        ' we need access to the "properties" button
        bOpen = hShowMacroControls()
        if ( not bOpen ) then
            warnlog( "Could not open macro controls, aborting" )
            hCloseBasicIde()
            brc = hDestroyDocument()
            goto endsub
        endif
        
        printlog( "Select every control, open its properties and verify its name, close properties" )
        for iCurrentControl = 1 to ICONTROLCOUNT

            if ( instr( gtSysName, "Solaris" ) > 0 ) then
                qaerrorlog( "Skipping frame control on Solaris" )
            else
                hSelectControl( iCurrentControl )
                hOpenPropertyBrowser()
                sCurrentControl = lsControlNames( iCurrentControl )
                hPBGetControlName( sCurrentControl )
                hClosePropertyBrowser()
            endif
            
        next iCurrentControl
        
        ' we need to delete the list-content otherwise we run into index-problems
        ListAllDelete( lsControlNames() )
       
        printlog( "Cleanup: Close the BASIC-IDE" )
        printlog( "Close the BASIC IDE" )
        hCloseBasicIde()        
        
        if ( getDocumentCount <> 1 ) then
            warnlog( "Number of open documents is incorrect, expected one only" )
        endif 

        printlog "Save the document"
        printlog( "save the document" )
        Call hFileSaveAsKill( sFile )

        printlog "Close it"
        printlog( "close the document" )
        brc = hDestroyDocument()

        printlog( "" ) 
        printlog( "Test case 2: Verify that all items keep their names after close and reload of the document" )
        
        printlog( "Open the file again" )
        printlog( "Open the document" )
        hFileOpen( sFile )

        printlog "Open tools/macro and select the last module for the current document"
        printlog( "open the basic-IDE of this document, open controls, activate 'Select-mode' and open the properties" )
        ToolsMacro_uno
        kontext "Makro"
        irc = hSelectNodeByName( MakroAus , CMODULE )
        if ( irc = 0 ) then
            qaerrorlog( "Searching node by name failed. Using fallback" )
            hSelectTheLastnode( MakroAus )
        else
            printlog( "The node was found. Good." )
        endif
        
        brc = hIsMacroEditButtonEnabled()
        if ( not brc ) then
            warnlog( "Edit-Button is not enabled, the test cannot continue" )
            kontext "Makro"
            Makro.cancel()
            brc = hDestroyDocument()
            goto endsub
        endif
            
        printlog( "Click 'Edit' to edit the module" )
        printlog( "Edit the module" )
        Bearbeiten.Click()
        if ( WaitSlot <> WSFinished ) then
            warnlog( "Slot not finished after 1 second" )
        endif

        kontext "BasicIDE"
        printlog( "Find the dialog we created before" )
        ' try to find our dialog again by stepping through the tabbar
        bOpen = hFindFirstDialog()
        if ( not bOpen ) then
            warnlog( "Could access dialog, aborting test" )
            hCloseBasicIde()
            brc = hDestroyDocument()
            goto endsub
        endif

        printlog( "Open the macro controls float" )
        ' if the dialog is open, open the macro controls toolbar as well,
        ' we need access to the "properties" button
        bOpen = hShowMacroControls()
        if ( not bOpen ) then
            warnlog( "Could not open macro controls, aborting" )
            hCloseBasicIde()
            brc = hDestroyDocument()
            goto endsub
        endif
        
        printlog( "Select every control, open its properties and verify its name, close properties" )
        for iCurrentControl = 1 to ICONTROLCOUNT

            if ( instr( gtSysName, "Solaris" ) > 0 ) then
                qaerrorlog( "Skipping frame control on Solaris" )
            else
                hSelectControl( iCurrentControl )
                hOpenPropertyBrowser()
                sCurrentControl = lsControlNames( iCurrentControl )
                hPBGetControlName( sCurrentControl )
                hClosePropertyBrowser()
            endif
            
        next iCurrentControl
        
        ' we need to delete the list-content otherwise we run into index-problems
        ListAllDelete( lsControlNames() )
       
        printlog( "Cleanup: Close the BASIC-IDE" )
        printlog( "Close the BASIC IDE" )
        hCloseBasicIde()
        
        printlog( "Cleanup: Close the document" )
        printlog( "Close the document" )
        brc = hDestroyDocument()
        
        printlog( "Cleanup: Delete the file we created" )
        hDeleteFile( sFile )
        
        printlog( "Repeat this for every application" )
        
    next iApplication

endcase


