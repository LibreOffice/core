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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : Basic form-controls
'*
'\******************************************************************************

private const ICONTROLCOUNT = 22

testcase tAllControlsOnDialog( cMajor as string )

    printlog( "Insert all available controls into a BASIC dialog and verify that changed settings persist a save/load cycle" )
    
    'qaerrorlog( "#i105685# - Test disabled" )
    'goto endsub

    dim iApplication as Integer ' numeric expression for gApplication
    dim cApplication as string  ' Name of the current application
    
    ' number of controls and the name of the current control
    dim iCurrentControl as integer
    dim sCurrentControl as string

    ' the workfile and the filter (Tested for 569, 645, 680)
    dim sFile as string

    const CMODULE = "tacod" ' abbreviation for tAllControlsOnDialog
    const BUTTON_TIMEOUT = -1
    const UPPER_BOUNDARY = 1 ' 1 = only writer, increase to add Calc, Impress, Draw...

    ' Currently set to test writer only. Increase upper boundary to add applications
    ' Up to 4 should be useful
    for iApplication = 1 to UPPER_BOUNDARY
    
        cApplication = hNumericDoctype( iApplication )
        printlog( "Document type: " & cApplication )

        sFile = convertpath( hGetWorkPath() & "basic" & hGetSuffix( cMajor ) )
        printlog( "Using file: " & sFile )
        
        printlog( "Delete the file, if it exists" )
        hDeleteFile( sFile )
                
        printlog( "Open a second document" )
        hCreateDocument()

        printlog( "Create a new module for the new document, new dialog, open macro controls" )
        if ( not hInitFormControls( CMODULE ) ) then
            printlog( "Aborting due to previous errors" )
            hDestroyDocument()
            goto endsub
        endif

        printlog( "Insert all controls into the dialog" )
        for iCurrentControl = 1 to ICONTROLCOUNT

            sCurrentControl = hDrawControlOnDialog( iCurrentControl )
            hUseAsyncSlot( "ContextProperties" )
            kontext "ControlPropertiesTabControl"
            if ( ControlPropertiesTabControl.exists( 1 ) ) then
                hPBSetControlName( sCurrentControl )
                hUseAsyncSlot( "ContextProperties" )
            else
                warnlog( "Property browser not open" )
            endif

        next iCurrentControl
        
        Kontext "BASICIDE"
        printlog( "close the BASIC-IDE" )
        hCloseBasicIde()
        
        printlog( "" )
        printlog( "Test case 1: Verify that settings survive a close and reopen of the BASIC IDE" )
        
        ToolsMacro_uno
        kontext "Makro"
        if ( hSelectNodeByName( MakroAus , CMODULE ) ) then
            printlog( "The node was found. Good." )
        else
            qaerrorlog( "Searching node by name failed. Using fallback" )
            hSelectTheLastnode( MakroAus )
        endif
        
        printlog( "Edit the module" )
        if ( hClickButton( Bearbeiten ) = BUTTON_TIMEOUT ) then
            warnlog( "Edit-Button is not enabled, the test cannot continue" )
            kontext "Makro"
            hCloseDialog( Makro, "cancel" )
            hDestroyDocument()
            goto endsub
        endif

        kontext "BasicIDE"
        printlog( "Find the dialog we created before" )
        if ( hFindFirstDialog() ) then
            printlog( "First dialog found. Good." )
        else
            warnlog( "#i115117 - Could not access dialog, accelerator not working? (Test interrupted)" )
            hCloseBasicIde()
            hDestroyDocument()
            goto endsub
        endif

        printlog( "Select every control, open its properties and verify its name, close properties" )
        for iCurrentControl = 1 to ICONTROLCOUNT

            sCurrentControl = hGetControlName( iCurrentControl )
            if ( hSelectControl( iCurrentControl ) ) then
                kontext "ControlPropertiesTabControl"
                if ( ControlPropertiesTabControl.exists( 5 ) ) then
                    wait( 300 )
                    hPBGetControlName( sCurrentControl )
                    hUseAsyncSlot( "ContextProperties" )
                    kontext "ControlPropertiesTabControl" : ControlPropertiesTabControl.notExists( 3 )
                else
                    warnlog( "Property browser not open for " & sCurrentControl )
                endif
            else
                warnlog( "Failed to select control: " & sCurrentControl )
            endif
            
        next iCurrentControl
       
        printlog( "Close the BASIC-IDE" )
        hCloseBasicIde()        
        
        if ( getDocumentCount <> 1 ) then warnlog( "Number of open documents is incorrect, expected one only" )

        printlog( "Save the document" )
        Call hFileSaveAsKill( sFile )

        printlog( "Close the document" )
        hDestroyDocument()

        printlog( "" ) 
        printlog( "Test case 2: Verify that all items keep their names after close and reload of the document" )
        
        printlog( "Open the file again" )
        hFileOpen( sFile )

        printlog( "Open the basic-IDE from this document, open controls, activate 'Select-mode' and open the properties" )
        ToolsMacro_uno

        kontext "Makro"
        if ( hSelectNodeByName( MakroAus , CMODULE ) ) then
            printlog( "The node was found. Good." )
        else
            qaerrorlog( "Searching node by name failed. Using fallback" )
            hSelectTheLastnode( MakroAus )
        endif
        
        printlog( "Edit the module" )
        if ( hClickButton( Bearbeiten ) = BUTTON_TIMEOUT ) then
            warnlog( "Edit-Button is not enabled, the test cannot continue" )
            kontext "Makro"
            hCloseDialog( Makro, "cancel" )
            hDestroyDocument()
            goto endsub
        endif

        kontext "BasicIDE"
        printlog( "Find the dialog we created before" )
        if ( hFindFirstDialog() ) then
            printlog( "First dialog found. Good." )
        else
            warnlog( "Could not access dialog, aborting test" )
            hCloseBasicIde()
            hDestroyDocument()
            goto endsub
        endif

        printlog( "Select every control, open its properties and verify its name, close properties" )
        for iCurrentControl = 1 to ICONTROLCOUNT

            sCurrentControl = hGetControlName( iCurrentControl )
            if ( hSelectControl( iCurrentControl ) ) then
                kontext "ControlPropertiesTabControl"
                if ( ControlPropertiesTabControl.exists( 5 ) ) then
                    wait( 300 )
                    hPBGetControlName( sCurrentControl )
                    hUseAsyncSlot( "ContextProperties" )
                    kontext "ControlPropertiesTabControl" : ControlPropertiesTabControl.notExists( 3 )
                else
                    warnlog( "Property browser not open for " & sCurrentControl )
                endif
            else
                warnlog( "Failed to select control: " & sCurrentControl )
            endif
            
        next iCurrentControl
            
        printlog( "Cleanup: Close the BASIC-IDE" )
        hCloseBasicIde()
        
        printlog( "Cleanup: Close the document" )
        hDestroyDocument()
        
        printlog( "Cleanup: Delete the file we created" )
        hDeleteFile( sFile )
        
    next iApplication

endcase


